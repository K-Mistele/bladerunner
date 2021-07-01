
import argparse

def get_file_bytes(fname):
	with open(fname, 'rb') as binary_file:
		file_bytes = binary_file.read()
		print(type(file_bytes))
		return file_bytes

# THE MAIN METHOD, MOSTLY HANDLES ARGUMENT PARSING AND KICKING OFF OTHER FUNCTIONS
def main():
	
	# ARGUMENT PARSING
	parser = argparse.ArgumentParser(description='A simple crypter to encode and encrypt binary shellcode payloads')
	parser.add_argument('--in', dest='in_file', required=True, help='Specify the input file (binary format required)')
	parser.add_argument('--out', dest='out_file', required=True, help='Specify the output file (binary or text format depending on encoding options')
	parser.add_argument('--bytewise-xor', dest='bytewise_xor', required=False, help='Specify the XOR key to encrypt the payload with. Acceptable values are 0-255')
	parser.add_argument('--xor-key', dest='xor_key', required=False, help='Specify a string to XOR encrypt the payload with, one character at a time')
	parser.add_argument('--encoding-type', dest='encoding_type', required=False, help='Specify an encoding type. Currently only base64 and base32 are supported')

	# BUILD ARGUMENTS AND CONVERT THE NAMESPACE INTO A DICT
	args = vars(parser.parse_args())
	print(args)

# RUN MAIN IF THIS IS LOADED AS THE MAIN MODULE
if __name__ == '__main__':
	main()