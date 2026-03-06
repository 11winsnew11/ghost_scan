default:
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/ripemd160.o -c hash/ripemd160.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/sha256.o -c hash/sha256.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/ripemd160_sse.o -c hash/ripemd160_sse.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/sha256_sse.o -c hash/sha256_sse.cpp
# 	g++ -O3 -o minikeyg minikeyg.cpp hash/ripemd160.o hash/ripemd160_sse.o hash/sha256.o hash/sha256_sse.o -lpthread
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c bloom/bloom.cpp -o bloom.o
	gcc -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-unused-parameter -Ofast -ftree-vectorize -c ecc/base58.c -o base58.o
	gcc -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Ofast -ftree-vectorize -c ecc/rmd160.c -o rmd160.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/sha3.c -o sha3.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/keccak.c -o keccak.o
	gcc -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Ofast -ftree-vectorize -c ecc/xhash.c -o xxhash.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c util.cpp -o util.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/Int.cpp -o Int.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/Point.cpp -o Point.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/SECP256K1.cpp -o SECP256K1.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -c ecc/IntMod.cpp -o IntMod.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c algo/Random.cpp -o Random.o
# 	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c algo/Mt32.cpp -o Mt32.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c algo/Xorshift32.cpp -o Xorshift32.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c algo/SPM32.cpp -o SPM32.o
# 	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c algo/HybridHex.cpp -o Twister.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -flto -c ecc/IntGroup.cpp -o IntGroup.o
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -o ecc/ripemd160.o -ftree-vectorize -flto -c ecc/ripemd160.cpp
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -o ecc/sha256.o -ftree-vectorize -flto -c ecc/sha256.cpp
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -o ecc/ripemd160_sse.o -ftree-vectorize -flto -c ecc/ripemd160_sse.cpp
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -o ecc/sha256_sse.o -ftree-vectorize -flto -c ecc/sha256_sse.cpp
	g++ -m64 -march=native -mtune=native -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize -o mk main.cpp base58.o rmd160.o ecc/ripemd160.o hash/ripemd160.o hash/ripemd160_sse.o hash/sha256.o hash/sha256_sse.o ecc/ripemd160_sse.o ecc/sha256.o ecc/sha256_sse.o SPM32.o Xorshift32.o bloom.o xxhash.o util.o Int.o  Point.o SECP256K1.o  IntMod.o  Random.o Mt32.o Twister.o IntGroup.o sha3.o keccak.o  -lm -lpthread
