#!/bin/bash

# Do all the annoying SSL key setup
###################################

# https://help.ubuntu.com/lts/serverguide/certificates-and-security.html

# Set any of these to non-empty to enable
ENCRYPTED_PRIVATE=
PLAIN_RSA_PUBLIC=
SELF_SIGN=yes

KEY_SIZE=2048
#!/bin/bash

# Do all the annoying SSL key setup
###################################

# https://help.ubuntu.com/lts/serverguide/certificates-and-security.html

# Set any of these to non-empty to enable
ENCRYPTED_PRIVATE=
PLAIN_RSA_PUBLIC=
SELF_SIGN=yes

KEY_SIZE=2048

if [ $# -ge 1 ]
then
	NAME=$1
else
	NAME=cert
fi

rm $NAME.*

# First generate the RSA private key
echo '[!!] Generating Private key [!!]'
if [ ! -z $ENCRYPTED_PRIVATE ]
then
	openssl genrsa -des3 -out $NAME.key $KEY_SIZE
else
	openssl genrsa -out $NAME.key $KEY_SIZE
fi

# Generate plain RSA public if requested
if [ ! -z $PLAIN_RSA_PUBLIC ]
then
	echo '[!!] Generating plain RSA public key [!!]'
	openssl rsa -in $NAME.key -outform PEM -out $NAME.pem
	exit
	# if plain RSA then done here
fi

# Generate X509 Signing request
echo '[!!] Generating Certificate signing request [!!]'
openssl req -new -key $NAME.key -outform PEM -out $NAME.csr

if [ ! -z $SELF_SIGN ]
then
	echo '[!!] Self-signing [!!]'
	openssl x509 -req -days 365 -in $NAME.csr -signkey $NAME.key -out $NAME.crt
fi
