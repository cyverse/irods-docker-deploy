#!/usr/bin/env python3

import hashlib
import binascii
import sys

if sys.argv[1].strip() == "":
    print("")
    exit(0)

salt = "CAD5089B"
password = sys.argv[1].strip()

salt_and_password = salt + password.encode('utf-8').hex()
salt_and_password = bytearray.fromhex(salt_and_password)
salted_sha256 = hashlib.sha256(salt_and_password).hexdigest()
password_hash = bytearray.fromhex(salt + salted_sha256)
password_hash = binascii.b2a_base64(password_hash).strip().decode('utf-8')

print(password_hash)
exit(0)