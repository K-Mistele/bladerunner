
import argparse
import base64

# BANNER
def banner():
	banner_str = """
	

                _|_
                 |
             .-'''''-.
          .-'    '-.
       .-'  :::::_:::::  '-.
   ___/ ==:...:::-:::...:== \___
  /_____________________________\
':'-._________________________.-'_
 ':::\ @-,`-[-][-^-][-]-`,-@ / _| |_
  '::| .-------------------. ||_ @ _|
   ::|=|*       Deep      *|=|'.| |
   ':| |'      Stone      '| |::.^|
   _:|=|'     Crypt       '|=|::::::.
 _| || |'        _        '| |:::::::.
|_   |=|'1634  _( )_  1789'|=|':::::.
  | || |' (   (_ ~ _)   ) '| | ':::'
  |^||=|*  )    (_)    (  *|=| '::'
     | '-------------------' .::::'
     |_____________________.::::::'
   .'___________________.::::::''
   |_______________.::::'':::'''
 .'_____________.::::::''::::''
            .:::''''   .'::::'
         .:::::''':.   .:::::'


	DeepStoneCrypt by @0xblacklight

	"""
	print(banner_str)

# READ IN A FILE IN BINARY AND RETURN THE BYTES
def get_file_bytes(fname):

	with open(fname, 'rb') as binary_file:
		return binary_file.read()

# WRITE OUT A TEXT FILE
def write_text_file(fname, contents):
	with open(fname, 'w') as text_file:
		text_file.write(contents)

# WRITE OUT A BINARY FILE
def write_binary_file(fname, contents):
	with open(fname, 'wb') as binary_file:
		binary_file.write(contents)

# PERFORM BYTEWISE XOR ENCRYPTION
def encrypt_bytewise_xor(shellcode, xor_key):

	# CONVERT STRING FROM PARAM TO INT AND BOUNDS CHECK IT
	try:
		xor_key = int(xor_key)
		if xor_key < 0 or xor_key > 255:
			raise ValueError("Invalid key size")
	except ValueError as e:
		print(f'[-] TypeError: {xor_key} is not an acceptable value (0-255) for an XOR key')
		quit(2)


	print(f'[+] Encrypting shellcode with key {xor_key}')
	encrypted_shellcode = b""

	# LOOP OVER AND PERFORM ENCRYPTION
	try:
		for i in range(0, len(shellcode)):
			encrypted_shellcode += bytes([shellcode[i] ^ xor_key])
	except Exception as e:
		print(f'[-] Error encrypting shellcode: {e}')
		quit(3)
	
		print(f'[+] Finished encrypting shellcode')
	return encrypted_shellcode

# PERFORM XOR KEY ENCRYPTION
def encrypt_xor_key(shellcode, key):

	print(f'[+] Encrypting shellcode with key {key}')

	encrypted_shellcode = b""
	i = 0 # INDEX IN src_bytes AND encrypted_bytes
	j = 0 # INDEX IN key
	while i < len(shellcode):

		# IF INDEX IN KEY IS PAST THE END, START AT THE BEGINNING OF THE KEY AGAIN
		if j >= len(key): j = 0

		# CREATE EACH ENCRYPTED BYTE BY XORING WITH SOME VALUE IN THE KEY
		encrypted_shellcode += bytes([shellcode[i] ^ord(key[j])])
		i += 1
		j += 1
	
	print(f'[+] Finished encrypting shellcode')
	return encrypted_shellcode

# THE MAIN METHOD, MOSTLY HANDLES ARGUMENT PARSING AND KICKING OFF OTHER FUNCTIONS
def main():
	
	banner()

	# ARGUMENT PARSING
	parser = argparse.ArgumentParser(description='A simple crypter to encode and encrypt binary shellcode payloads')
	parser.add_argument('--in', dest='in_file', required=True, help='Specify the input file (binary format required)')
	parser.add_argument('--out', dest='out_file', required=True, help='Specify the output file (binary or text format depending on encoding options')
	parser.add_argument('--bytewise-xor', dest='bytewise_xor', required=False, help='Specify the XOR key to encrypt the payload with. Acceptable values are 0-255')
	parser.add_argument('--xor-key', dest='xor_key', required=False, help='Specify a string to XOR encrypt the payload with, one character at a time')
	parser.add_argument('--encoding-type', dest='encoding_type', required=False, choices=['base64', 'base32'], help='Specify an encoding type. Currently only base64 and base32 are supported')

	# BUILD ARGUMENTS AND CONVERT THE NAMESPACE INTO A DICT
	args = vars(parser.parse_args())
	print(f'[+] Debug: {args}')

	# VALIDATE ARGUMENTS
	if (args['bytewise_xor'] and args['xor_key']):
		print('[-] Error: only one type of encryption may be specified!')
		quit(1)

	# READ IN SHELLCODE
	try:
		shellcode = get_file_bytes(args['in_file'])
	except Exception as e:
		print(f'[-] Error trying to read in shellcode file: {e}')
	print(f'[+] Read in {len(shellcode)} bytes of shellcode from file successfully')

	# DEFINE VARS
	encrypted_shellcode = b''

	# DETERMINE ENCRYPTION TYPE IF IT EXISTS AND PERFORM ENCRYPTION
	if args['bytewise_xor']:
		print(f'[+] Bytewise XOR encryption selected. Proceeding with encryption.')
		encrypted_shellcode = encrypt_bytewise_xor(shellcode, args['bytewise_xor'])
	elif args['xor_key']:
		print(f'[+] XOR key encryption selected. Proceeding with encryption.')
		encrypted_shellcode = encrypt_xor_key(shellcode, args['xor_key'])
	else:
		print(f'[+] No encryption selected, proceeding to encoding')
		encrypted_shellcode = shellcode


	# DETERMINE ENCODING TYPE IF IT EXISTS AND PERFORM ENCODING
	encoded_shellcode = ""
	if args["encoding_type"] == 'base64':
		print(f'[+] Base64 encoding selected (UTF-8)')
		encoded_shellcode = (base64.b64encode(encrypted_shellcode)).decode('utf-8')
	elif args['encoding_type'] == 'base32':
		print(f'[+] Base32 encoding selected (UTF-8)')
		encoded_shellcode = (base64.b16encode(encrypted_shellcode)).decode('utf-8')
	else:
		print(f'[+] No encoding selected.')
		encoded_shellcode = encrypted_shellcode
	

	# WRITE OUTPUT
	if args['encoding_type']:
		print(f'[+] Writing out {len(encoded_shellcode)} characters to {args["out_file"]}')
		write_text_file(args['out_file'], encoded_shellcode)
		print(f'[+] Finished writing out {args["out_file"]}')
	else:
		print(f'[+] Writing out {len(encoded_shellcode)} bytes to {args["out_file"]}')
		write_binary_file(args['out_file'], encoded_shellcode)
		print(f'[+] Finished writing out {args["out_file"]}')
	print(f'[+] Hack the Planet!')
	
	

# RUN MAIN IF THIS IS LOADED AS THE MAIN MODULE
if __name__ == '__main__':
	main()
