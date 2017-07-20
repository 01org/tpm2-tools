#;**********************************************************************;
#
# Copyright (c) 2016, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, 
# this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, 
# this list of conditions and the following disclaimer in the documentation 
# and/or other materials provided with the distribution.
#
# 3. Neither the name of Intel Corporation nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
# THE POSSIBILITY OF SUCH DAMAGE.
#;**********************************************************************;
#!/bin/bash

 nv_test_index=0x1500018
 nv_auth_handle=0x40000001

tpm2_takeownership -c 
 if [ $? != 0 ];then 
 echo "clean ownership Fail!"
 exit 1
 fi
tpm2_nvlist|grep -i $nv_test_index
if [ $? = 0 ];then
tpm2_nvrelease -x $nv_test_index -a $nv_auth_handle 
 if [ $? != 0 ];then 
 echo "please release the nv index $nv_test_index first!"
 exit 1
 fi
fi

tpm2_nvdefine -x $nv_test_index -a $nv_auth_handle -s 32 -t 0x2000A  
if [ $? != 0 ];then 
 echo "nvdefine fail,Please check your environment!"
 exit 1
fi


if [ ! -f nv.test_w ];then
 echo "please123abc" >nv.test_w
fi

tpm2_nvwrite -x $nv_test_index -a $nv_auth_handle  -f nv.test_w 
if [ $? != 0 ];then 
 echo "nvwrite fail!"
 exit 1
fi

tpm2_nvread -x $nv_test_index -a $nv_auth_handle  -s 32 -o 0

if [ $? != 0 ];then 
 echo "nvread fail!"
 exit 1
fi

tpm2_nvlist|grep -i $nv_test_index
if [ $? != 0 ];then 
 echo "nvlist  fail or double check the define index!"
 exit 1
fi

tpm2_nvrelease -x $nv_test_index -a $nv_auth_handle  

if [ $? != 0 ];then 
 echo "nvrelease  fail or double check the define index!"
 exit 1
else
 echo "release the nv index OK!"
fi


echo "f28230c080bbe417141199e36d18978228d8948fc10a6a24921b9eba6bb1d988" \
| xxd -r -p > policy.bin

tpm2_nvdefine -x 0x1500016 -a 0x40000001 -s 32 -t 0x2000A -L policy.bin -r -w
if [ $? != 0 ];then
 echo "Failed tpm2_nvdefine"
 exit 1
fi

tpm2_nvlist | grep 0x1500016 -A5 | grep Auth | grep -o ": [a-zA-Z0-9]\{1,\}" | \
grep -o "[a-zA-Z0-9]\{1\}" | xxd -r -p >test.bin
if [ $? != 0 ];then
 echo "Failed tpm2_nvlist"
 exit 1
fi

cmp test.bin policy.bin -s
if [ $? != 0 ];then
 echo "nvdefine with policy in authorization structure failed"
 exit 1
fi

tpm2_nvrelease -x 0x1500016 -a 0x40000001
if [ $? != 0 ];then
 echo "nvrelease failed"
 exit 1
fi

rm -f policy.bin test.bin nv.test_w

#
# Test large writes
#
# The simulator caps us at 2048, is there a way to query this max size?
large_file_size=2047
nv_test_index=0x1000000

# Create an nv space with attributes 1010 = TPMA_NV_PPWRITE and TPMA_NV_AUTHWRITE
tpm2_nvdefine -x $nv_test_index -a $nv_auth_handle -s $large_file_size -t 0x2000A
if [ $? != 0 ];then
 echo "nvdefine failed for a size of $large_file_size"
 exit 1
fi

large_file_name="nv.test_large_w"

if [ ! -f $large_file_name ]; then
  base64 /dev/urandom | head -c $large_file_size > $large_file_name
fi

tpm2_nvwrite -x $nv_test_index -a $nv_auth_handle  -f $large_file_name
if [ $? != 0 ];then
  rm -f $large_file_name
  echo "nvwrite failed for testing large writes!"
  exit 1
fi

#
# TODO: Reading large files is currently broken, add a test and ensure the written matches
# the read.
#

rm -f $large_file_name

tpm2_nvlist|grep -i $nv_test_index
if [ $? != 0 ];then
  echo "nvlist  fail or double check the define index!"
  exit 1
fi

tpm2_nvrelease -x $nv_test_index -a $nv_auth_handle

exit 0
