# Diamond Grid Cipher

A C++ encryption and decryption program built around a custom diamond-spiral grid cipher.

Plaintext is sanitised, written through concentric diamond paths inside an odd-sized square grid, padded with random uppercase characters, and serialised column-by-column to produce the ciphertext.

The program supports both single-round and multi-round encryption and can reverse the process to recover the original message.

## Features

- automatic odd grid-size calculation
- manual grid-size selection
- multi-round encryption
- multi-round decryption
- random padding for unused cells
- input sanitisation
- validation for grid and ciphertext sizes
- interactive terminal menu
- visual grid output for each encryption/decryption round

## Cipher flow

```text
Plaintext
   |
Sanitise input
   |
Choose odd grid size
   |
Fill concentric diamond paths
   |
Fill remaining cells with random letters
   |
Read grid column-by-column
   |
Ciphertext
```

## Build

```bash
g++ -std=c++17 diamond_cipher.cpp -O2 -o diamond_cipher
```

## Run

```bash
./diamond_cipher
```

The interactive menu provides options for encrypting and decrypting messages and selecting one or multiple rounds.

## Stack

- C++17
- object-oriented design
- grid traversal
- input validation
- random number generation
- custom encryption/decryption logic

## Note

This is a custom educational cipher design and is not intended for production cryptographic security.
