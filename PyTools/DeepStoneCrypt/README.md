# DeepStoneCrypt
DeepStoneCrypt is a payload crypter and encoder. Requires Python >= 3.6

## Usage
* `--in`: the input binary file
* `--out`: the output file. This will be a binary file if no payload encoding is specified, otherwise it will be a text file.
* `--xor-key`: Use a key string to XOR against the payload repeatedle
* `--bytewise-xor`: XOR all bytes in the payload against a single byte. Note that the argument is interpreted as decimal, not hex.
* `--encoding-type`: specify the encoding type. Currently supports `base64` and `base32`

Notes: 
* only one encryption type may be used at a time
* only on encoding type may be specified
* encoding type is optional
* encryption is optional

```

python DeepStoneCrypt.py --in payload.bin --out payload.txt --xor-key EncryptionKey12 --encoding-type base64

python DeepStoneCrypt.py --in payload.bin --out payload.txt --bytewise-xor 33 --encoding-type base32

python DeepStoneCrypt.py --in payload.bin --out payload_encr.bin --xor-key SuperSecureKey

python DeepStoneCrypt.py --in payload.bin --out payload.txt --encoding-type base64
```