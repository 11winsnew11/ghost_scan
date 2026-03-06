/*
Develop by Winsnew
email: winsnew@gmail.com
Modified for MiniKey Search Mode
Searches valid Casascius Mini Keys matching target addresses
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <inttypes.h>
#include <string>
#include <sstream>
#include "ecc/libbase58.h"
#include "ecc/rmd160.h"
#include "bloom/bloom.h"
#include "ecc/sha3.h"
#include "util.h"
#include "ecc/SECP256K1.h"
#include "ecc/Point.h"
#include "ecc/Int.h"
#include "ecc/IntGroup.h"
#include "ecc/sha256.h"
#include "ecc/ripemd160.h"
#include <thread>
#include <functional>

#if defined(_WIN64) && !defined(__CYGWIN__)
#include "getopt.h"
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/random.h>
#endif

#ifdef __unix__
#ifdef __CYGWIN__
#else
#include <linux/random.h>
#endif
#endif

#define MODE_MINIKEY 7
#define SEARCH_UNCOMPRESS 0
#define SEARCH_COMPRESS 1
#define SEARCH_BOTH 2

uint32_t THREADBPWORKLOAD = 1048576;

struct thread_data
{
    int thread_id;
    uint64_t *steps;
    unsigned int *ends;
};

// Struct untuk Public Key (tidak banyak berubah)
#if defined(_WIN64) && !defined(__CYGWIN__)
#pragma pack(push, 1)
struct publickey
{
    uint8_t parity;
    union
    {
        uint8_t data8[32];
        uint32_t data32[8];
        uint64_t data64[4];
    } X;
};
#pragma pack(pop)
#else
struct publickey
{
    uint8_t parity;
    union
    {
        uint8_t data8[32];
        uint32_t data32[8];
        uint64_t data64[4];
    } X;
} __attribute__((packed));
#endif

const char *Ccoinbuffer_default = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
char *Ccoinbuffer = (char *)Ccoinbuffer_default;
const char *version = "0.3.240519 MiniKey Search Mode";

const char *publicsearch[3] = {"uncompress", "compress", "both"};
const char *default_fileName = "targets.txt";

#if defined(_WIN64) && !defined(__CYGWIN__)
HANDLE *tid = NULL;
HANDLE write_keys;
HANDLE minikey_mutex; // Mutex untuk increment minikey global
#else
pthread_t *tid = NULL;
pthread_mutex_t write_keys;
pthread_mutex_t minikey_mutex; // Mutex untuk increment minikey global
#endif

// Variabel Global
int NTHREADS = 1;
int FLAGSEARCH = SEARCH_BOTH; // Default cari keduanya
int FLAGQUIET = 0;
Int OUTPUTSECONDS;
Int ZERO;
uint64_t *steps = NULL;
unsigned int *ends = NULL;
Secp256K1 *secp;
struct bloom bloom_target;
uint64_t total_targets = 0;

// Variabel Global untuk Mini Key State
char global_minikey[40];       // Buffer string minikey global
unsigned char global_raw_minikey[40]; // Buffer index base58 global
int MINIKEY_SIZE = 22;         // Default 22 karakter (S + 21 char)
int INCREMENT_OFFSET = 21;     // Index karakter terakhir

// Prototipe Fungsi
void menu();
void sleep_ms(int milliseconds);
bool initBloomFilter(struct bloom *bloom_arg, uint64_t items_bloom);
void checkpointer(void *ptr, const char *file, const char *function, const char *name, int line);
bool readTargetsFile(char *fileName);
void writeFoundKey(bool compressed, Int *key, char *minikey_str);
bool increment_minikey_index(char *buffer, unsigned char *rawbuffer, int index);
bool isValidMiniKey(const char *minikey, unsigned char *privkey_hash);
void init_minikey();

#if defined(_WIN64) && !defined(__CYGWIN__)
DWORD WINAPI thread_process_minikey(LPVOID vargp);
#else
void *thread_process_minikey(void *vargp);
#endif

// Fungsi utilitas untuk mengubah Address ke Hash160 (untuk bloom filter)
bool addressToHash160(const char *address, unsigned char *hash160_out) {
    uint8_t bin[25]; // Address base58 biasanya 25 byte (1 version + 20 hash + 4 checksum)
    size_t bin_len = 25;
    
    if (!b58tobin(bin, &bin_len, address, strlen(address))) {
        return false;
    }
    
    // b58tobin mengisi dari belakang. Jika hasilnya 25 byte, berarti valid.
    if (bin_len != 25) return false;
    
    // Hash160 ada di byte 1 sampai 20
    memcpy(hash160_out, bin + 1, 20);
    return true;
}

int main(int argc, char **argv)
{
    char *fileName = NULL;
    char *hextemp = NULL;
    int c, j, s;
    
    ZERO.SetInt32(0);
    OUTPUTSECONDS.SetInt32(30);

    #if defined(_WIN64) && !defined(__CYGWIN__)
        write_keys = CreateMutex(NULL, FALSE, NULL);
        minikey_mutex = CreateMutex(NULL, FALSE, NULL);
    #else
        pthread_mutex_init(&write_keys, NULL);
        pthread_mutex_init(&minikey_mutex, NULL);
    #endif

    srand(time(NULL));
    secp = new Secp256K1();
    secp->Init();

    // Seed random
    #if defined(_WIN64) && !defined(__CYGWIN__)
        rseed(clock() + time(NULL) + rand());
    #else
        unsigned long rseedvalue;
        getrandom(&rseedvalue, sizeof(unsigned long), GRND_NONBLOCK);
        rseed(rseedvalue);
    #endif

    printf("[+] Version %s\n", version);

    while ((c = getopt(argc, argv, "hqf:t:s:l:")) != -1)
    {
        switch (c)
        {
        case 'h':
            menu();
            break;
        case 'f':
            fileName = optarg;
            break;
        case 't':
            NTHREADS = strtol(optarg, NULL, 10);
            if (NTHREADS <= 0) NTHREADS = 1;
            printf("[+] Threads : %u\n", NTHREADS);
            break;
        case 's':
            OUTPUTSECONDS.SetBase10(optarg);
            if (OUTPUTSECONDS.IsLower(&ZERO)) OUTPUTSECONDS.SetInt32(30);
            break;
        case 'l':
            if (strcmp(optarg, "compress") == 0) FLAGSEARCH = SEARCH_COMPRESS;
            else if (strcmp(optarg, "uncompress") == 0) FLAGSEARCH = SEARCH_UNCOMPRESS;
            else FLAGSEARCH = SEARCH_BOTH;
            printf("[+] Search mode: %s\n", optarg);
            break;
        case 'q':
            FLAGQUIET = 1;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    // Inisialisasi Mini Key Global
    init_minikey();

    // Load Targets
    if (fileName == NULL) fileName = (char *)default_fileName;
    
    if (!readTargetsFile(fileName)) {
        fprintf(stderr, "[E] No targets loaded. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    steps = (uint64_t *)calloc(NTHREADS, sizeof(uint64_t));
    ends = (unsigned int *)calloc(NTHREADS, sizeof(int));
    
    #if defined(_WIN64) && !defined(__CYGWIN__)
        tid = (HANDLE *)calloc(NTHREADS, sizeof(HANDLE));
    #else
        tid = (pthread_t *)calloc(NTHREADS, sizeof(pthread_t));
    #endif

    struct thread_data *thread_data_array = (struct thread_data *)malloc(NTHREADS * sizeof(struct thread_data));

    printf("[+] Starting MiniKey Search...\n");

    for (j = 0; j < NTHREADS; j++)
    {
        thread_data_array[j].thread_id = j;
        thread_data_array[j].steps = steps;
        thread_data_array[j].ends = ends;
        
        #if defined(_WIN64) && !defined(__CYGWIN__)
            tid[j] = CreateThread(NULL, 0, thread_process_minikey, &thread_data_array[j], 0, NULL);
        #else
            s = pthread_create(&tid[j], NULL, thread_process_minikey, &thread_data_array[j]);
        #endif
    }

    // Loop Statistik
    Int total, pretotal, seconds;
    total.SetInt32(0);
    seconds.SetInt32(0);
    int continue_flag = 1;

    do {
        sleep_ms(1000);
        seconds.AddOne();
        
        // Cek jika semua thread selesai (seharusnya tidak pernah selesai kecuali error)
        int check_flag = 1;
        for(j=0; j<NTHREADS; j++) if(!ends[j]) check_flag = 0;
        if(check_flag) continue_flag = 0;

        if (OUTPUTSECONDS.IsGreater(&ZERO))
        {
            Int MPZAUX;
            MPZAUX.Set(&seconds);
            MPZAUX.Mod(&OUTPUTSECONDS);
            if (MPZAUX.IsZero())
            {
                total.SetInt32(0);
                for (j = 0; j < NTHREADS; j++)
                {
                    // steps[j] berapa batch yang sudah diproses
                    // Asumsi 1 batch = 1 kunci dicoba (disimplifikasi)
                    total.Add(steps[j]); 
                }
                
                char *str_total = total.GetBase10();
                char *str_sec = seconds.GetBase10();
                printf("\r[Stats] Checked: %s keys in %s seconds    ", str_total, str_sec);
                fflush(stdout);
                free(str_total);
                free(str_sec);
            }
        }
    } while (continue_flag);

    printf("\nEnd\n");
    return 0;
}

void menu()
{
    printf("\nUsage:\n");
    printf("-h          Show help\n");
    printf("-f file     File with target addresses (one per line)\n");
    printf("-t num      Number of threads\n");
    printf("-s sec      Stats output interval\n");
    printf("-l mode     Address type: compress, uncompress, both\n");
    printf("-q          Quiet output\n");
    printf("\nExample:\n");
    printf("./bitghost -f targets.txt -t 4\n");
    exit(0);
}

// Inisialisasi Mini Key Global dengan nilai acak
void init_minikey() {
    FILE *fd = fopen("/dev/urandom", "r");
    if (fd) {
        fread(global_raw_minikey, sizeof(char), MINIKEY_SIZE, fd);
        fclose(fd);
    } else {
        for(int i=0; i<MINIKEY_SIZE; i++) global_raw_minikey[i] = rand();
    }

    global_minikey[0] = 'S'; // Mini key selalu mulai dengan S
    global_raw_minikey[0] = 25; // Index karakter 'S' pada Base58

    int ccoinbuffer_len = strlen(Ccoinbuffer);

    for (int i = 1; i < MINIKEY_SIZE; i++) {
        global_raw_minikey[i] = global_raw_minikey[i] % ccoinbuffer_len;
        global_minikey[i] = Ccoinbuffer[global_raw_minikey[i]];
    }
    global_minikey[MINIKEY_SIZE] = '\0';
    printf("[+] Initial MiniKey: %s\n", global_minikey);
}

// Fungsi Increment dari minikey.cpp
bool increment_minikey_index(char *buffer, unsigned char *rawbuffer, int index)
{
    int ccoinbuffer_len = strlen(Ccoinbuffer); // 58
    if (rawbuffer[index] < ccoinbuffer_len - 1)
    {
        rawbuffer[index]++;
        buffer[index] = Ccoinbuffer[rawbuffer[index]];
    }
    else
    {
        rawbuffer[index] = 0x00;
        buffer[index] = Ccoinbuffer[0];
        if (index > 1) // Jangan increment index 0 ('S')
        {
            return increment_minikey_index(buffer, rawbuffer, index - 1);
        }
        else
        {
            return false; // Overflow
        }
    }
    return true;
}

// Cek Validitas Mini Key
// Mini Key valid jika SHA256(minikey + "?")[0] == 0x00
bool isValidMiniKey(const char *minikey, unsigned char *privkey_hash)
{
    char buffer[40];
    unsigned char hash[32];

    strcpy(buffer, minikey);
    strcat(buffer, "?");

    sha256((unsigned char *)buffer, strlen(buffer), hash);

    if (hash[0] == 0x00)
    {
        // Jika valid, hitung private key asli (SHA256 tanpa "?")
        sha256((unsigned char *)minikey, strlen(minikey), privkey_hash);
        return true;
    }
    return false;
}

// Thread Worker
#if defined(_WIN64) && !defined(__CYGWIN__)
DWORD WINAPI thread_process_minikey(LPVOID vargp)
#else
void *thread_process_minikey(void *vargp)
#endif
{
    struct thread_data *data = (struct thread_data *)vargp;
    int thread_id = data->thread_id;
    uint64_t *steps = data->steps;

    char local_minikey[40];
    unsigned char local_raw_minikey[40];
    
    unsigned char privkey_bytes[32];
    unsigned char pubKeyHash[20];
    
    Int keyInt;
    Point publicKey;

    while (1)
    {
        // 1. Ambil kunci saat ini (Lock)
        #if defined(_WIN64) && !defined(__CYGWIN__)
            WaitForSingleObject(minikey_mutex, INFINITE);
        #else
            pthread_mutex_lock(&minikey_mutex);
        #endif

        memcpy(local_minikey, global_minikey, MINIKEY_SIZE + 1);
        memcpy(local_raw_minikey, global_raw_minikey, MINIKEY_SIZE);
        
        // Increment global key untuk iterasi selanjutnya
        increment_minikey_index(global_minikey, global_raw_minikey, INCREMENT_OFFSET);

        #if defined(_WIN64) && !defined(__CYGWIN__)
            ReleaseMutex(minikey_mutex);
        #else
            pthread_mutex_unlock(&minikey_mutex);
        #endif

        // 2. Validasi Mini Key (Checksum)
        if (isValidMiniKey(local_minikey, privkey_bytes))
        {
            // Konversi byte ke Int
            keyInt.Set32Bytes(privkey_bytes);

            // 3. Hitung Public Key
            // Kita butuh hash160. 
            // Bitghost memiliki secp->GetHash160 atau bisa manual.
            
            // Gunakan fungsi manual atau secp wrapper
            // Asumsi FLAGSEARCH menentukan apakah kita hitung Y atau tidak.
            // Untuk simplifikasi, hitung hash saja.
            
            publicKey = secp->ComputePublicKey(&keyInt);

            // Cek Compressed
            if (FLAGSEARCH == SEARCH_COMPRESS || FLAGSEARCH == SEARCH_BOTH)
            {
                secp->GetHash160(P2PKH, true, publicKey, pubKeyHash);
                if (bloom_check(&bloom_target, pubKeyHash, 20))
                {
                    writeFoundKey(true, &keyInt, local_minikey);
                }
            }

            // Cek Uncompressed
            if (FLAGSEARCH == SEARCH_UNCOMPRESS || FLAGSEARCH == SEARCH_BOTH)
            {
                secp->GetHash160(P2PKH, false, publicKey, pubKeyHash);
                if (bloom_check(&bloom_target, pubKeyHash, 20))
                {
                    writeFoundKey(false, &keyInt, local_minikey);
                }
            }
        }

        // Update counter stats
        steps[thread_id]++;
    }
    return NULL;
}

void writeFoundKey(bool compressed, Int *key, char *minikey_str)
{
    Point pubKey = secp->ComputePublicKey(key);
    char pubKeyHex[131];
    secp->GetPublicKeyHex(compressed, pubKey, pubKeyHex);

    // Generate Address
    char address[50];
    unsigned char hash160[20];
    secp->GetHash160(P2PKH, compressed, pubKey, hash160);
    
    // Convert Hash160 to Address
    // Bitghost punya rmd160toaddress_dst
    rmd160toaddress_dst((char*)hash160, address);

    #if defined(_WIN64) && !defined(__CYGWIN__)
        WaitForSingleObject(write_keys, INFINITE);
    #else
        pthread_mutex_lock(&write_keys);
    #endif

    FILE *f = fopen("MINIKEYFOUND.txt", "a");
    if (f) {
        fprintf(f, "MiniKey: %s\nPrivKey: %s\nPubKey: %s\nAddress: %s\n\n", 
                minikey_str, key->GetBase16(), pubKeyHex, address);
        fclose(f);
    }
    
    printf("\n\n[!!!] FOUND MATCH!\nMiniKey: %s\nAddress: %s\n\n", minikey_str, address);

    #if defined(_WIN64) && !defined(__CYGWIN__)
        ReleaseMutex(write_keys);
    #else
        pthread_mutex_unlock(&write_keys);
    #endif
}

bool readTargetsFile(char *fileName)
{
    FILE *fd = fopen(fileName, "r");
    if (!fd)
    {
        fprintf(stderr, "[E] Cannot open file: %s\n", fileName);
        return false;
    }

    // Hitung jumlah target dulu (sederhana: realloc)
    char line[100];
    uint64_t count = 0;
    std::vector<std::string> addresses;

    while (fgets(line, sizeof(line), fd))
    {
        // Trim newline
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) > 20 && line[0] == '1') // Simple Bitcoin address check
        {
            addresses.push_back(line);
            count++;
        }
    }
    fclose(fd);

    if (count == 0) return false;

    printf("[+] Loaded %llu target addresses.\n", count);

    // Init Bloom Filter
    if (!initBloomFilter(&bloom_target, count)) return false;

    unsigned char hash160[20];
    for (auto addr : addresses)
    {
        if (addressToHash160(addr.c_str(), hash160))
        {
            bloom_add(&bloom_target, hash160, 20);
        }
        else
        {
            fprintf(stderr, "[W] Invalid address skipped: %s\n", addr.c_str());
        }
    }

    printf("[+] Bloom filter initialized.\n");
    return true;
}

bool initBloomFilter(struct bloom *bloom_arg, uint64_t items_bloom)
{
    // Copy paste dari kode asli
    if (items_bloom <= 10000) items_bloom = 10000;
    if (bloom_init2(bloom_arg, items_bloom, 0.000001) == 1)
    {
        fprintf(stderr, "[E] Bloom init failed\n");
        return false;
    }
    return true;
}

void sleep_ms(int milliseconds)
{
    #if defined(_WIN64) && !defined(__CYGWIN__)
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}

void checkpointer(void *ptr, const char *file, const char *function, const char *name, int line)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "[E] Null pointer: %s at %s:%d\n", name, file, line);
        exit(1);
    }
}

// Helper function to convert RMD160 to Address (needed for output)
// Implementasi sederhana menggunakan libbase58
void rmd160toaddress_dst(char *rmd, char *dst)
{
    char digest[60];
    size_t pubaddress_size = 40;
    digest[0] = 0x00; // Bitcoin Mainnet
    memcpy(digest + 1, rmd, 20);
    sha256((uint8_t *)digest, 21, (uint8_t *)digest + 21);
    sha256((uint8_t *)digest + 21, 32, (uint8_t *)digest + 21);
    if (!b58enc(dst, &pubaddress_size, digest, 25))
    {
        fprintf(stderr, "error b58enc\n");
    }
}