#!/bin/bash

# Check version of OS.
recommendedOS="CentOS Linux release 7.6.1810 (Core) "
currentOS=$(cat /etc/centos-release)
if [ "$currentOS" != "$recommendedOS" ]
then
	echo "[WARNING] You are not building on $recommendedOS."
fi

# Build implementation library.
make -C ./src

if [ ! -f "./validate" ]
then
   echo "There were errors during compilation."
   exit 1
fi

echo "Attempting to produce validation output..."

export LD_LIBRARY_PATH=.

./validate > ./validation.txt

if [[ $? != 0 ]]
then
   echo "[ERROR] Validation output not produced."
	exit 1
fi

echo "[SUCCESS]"

# Create submission tar file.
echo "Creating submission package."

tar -czvf libIREX10.tar.gz ./config ./libIREX10.so ./validation.txt

echo "[SUCCESS]

A submission package has been generated (libIREX10.tar.gz). 

You must encrypt and sign this file before transmitting it to NIST. Please follow the
encryption instructions at https://www.nist.gov/sites/default/files/nist_encryption.pdf
using the public key 'irex.asc'.

For example:
      gpg --default-key <ParticipantEmail> --encrypt --recipient irex@nist.gov --sign \\
          --output libIREX_1N_<ParticipantName>_<SubmissionNumber>.gpg libIREX10.tar.gz

If the encrypted file is 22 Mb or less you may send it to irex@nist.gov as an attachment.
Alternatively, you can provide a download link from a webserver (e.g., a Google Drive link)."

