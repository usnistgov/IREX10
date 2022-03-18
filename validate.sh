#!/bin/bash

# Check version of OS.

reqOS="Ubuntu 20.04.3 LTS"
currentOS=$(lsb_release -a 2> /dev/null | grep "Description" | awk -F":" '{ print $2 }' | sed -e 's/^[[:space:]]*//')

if [ "$reqOS" != "$currentOS" ]
then
   echo "${bold}[ERROR] You are not running the correct version of the operating system, which should be $reqOS.  Please install the correct operating system and re-run this validation package.${normal}"
   exit
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

