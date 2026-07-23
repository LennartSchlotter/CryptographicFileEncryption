# Cryptographic File Encryption Tool
A CLI tool that supports encrypting and decrypting of files. Standalone C++-CLI project primarily serving as a learning opportunity for both the language as well as cryptographic basics. 

Please keep in mind that this is a learning project. While care has been taken in its design and this project is open for maintenance and improvements, it has not undergone professional security auditing. It should not be relied upon for protecting highly sensitive data.

## Features
- file encryption and decryption
    - password-based, symmetric
    - key-based, asymmetric (hybrid)
        > [This is implemented in a hybrid fashion. Encryption is still done symmetrically, but with an asymmetrically encrypted key.]
- Key generation for asymmetric algorithms

## Usage
### Installation
Prerequisites:
- C++ compiler (g++ or clang++)
- CMake
- (Optional) Git

Build from Source:
```bash
git clone https://github.com/LennartSchlotter/CryptographicFileEncryption.git
cd CryptographicFileEncryption

make release
```

### Usage
```bash
cfe <command> [options]
```
You may run `cfe --help` for an overview of the functionalities.

## Internal Structure
The file created during encryption has a `.cfe` extension. This is a custom file-type that is used so that the program can store non-sensitive data for the encryption process. As such, the structure of the header is standardized to allow parsing and the magic bytes are verified prior to decryption.

### Magic Bytes
Files encrypted with this tool begin with the following 8 magic bytes:
`43 46 45 2A 5F 43 4C 49`

### Header
Each file contains important metadata used by the program for decryption. These are versioned through the `Format Version` field and as such backward-compatible.
Due to differing demands by the decryption algorithms, the structure differs between asymmetric and symmetric algorithms.

#### Symmetric
| Offset  | Size  | Field |
|---------|-------|-------|
| 0       | 8     | Magic Bytes |
| 8       | 1     | Format Version |
| 9       | 1     | Algorithm ID |
| 10      | 12    | Salt |
| 22      | 4     | Time Cost |
| 26      | 4     | Memory Cost |
| 30      | 1     | Parallelism |
| 31      | 12    | IV |
| 43      | 8     | Ciphertext Length |
| 51      | n     | Ciphertext |
| 51+n    | 16    | Auth tag |

#### Asymmetric
| Offset  | Size  | Field |
|---------|-------|-------|
| 0       | 8     | Magic Bytes |
| 8       | 1     | Format Version |
| 9       | 1     | Algorithm ID |
| 10      | 4     | Encrypted Key Length |
| 14      | m    | Encrypted Session Key |
| 14+m      | 12    | IV |
| 26+m      | 8     | Ciphertext Length |
| 34+m      | n     | Ciphertext |

## Additional Information
As this is primarily a learning project, information on the development process can be found in `Development.md`
