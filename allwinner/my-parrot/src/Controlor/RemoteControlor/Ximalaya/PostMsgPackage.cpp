/*
 * PostMsgPackage.cpp
 *
 *  Created on: 2016年5月18日
 *      Author: Administrator
 */

#include "PostMsgPackage.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <openssl/md5.h>
#include <ctime>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>

static int base64_encode(unsigned char *str, int str_len, unsigned char *encode, int encode_len)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, str, str_len); //encode
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    if (bptr->length > encode_len)
    {
        return -1;
    }
    encode_len = bptr->length;
    memcpy(encode, bptr->data, bptr->length);
    BIO_free_all(b64);
    return encode_len;
}

static void sha1_hmac(unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[20])
{
    unsigned int len = 20;

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, keylen, EVP_sha1(), NULL);
    HMAC_Update(&ctx, input, ilen);
    HMAC_Final(&ctx, output, &len);
    HMAC_CTX_cleanup(&ctx);
}

static void split(const string& src, const string& separator, vector<string>& dest)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;

    do
    {
        index = str.find_first_of(separator, start);
        if (index != string::npos)
        {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if (start == string::npos)
                return;
        }
    } while (index != string::npos);

    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
}

static string mapToPostStr(map<string, string> & myMap)
{
    string result = "";
    for (map<string, string>::iterator it = myMap.begin(); it != myMap.end(); ++it)
    {
        result += it->first + "=" + it->second + "&";
    }

    result = result.substr(0, result.length() - 1);

    return result;
}

static string getSig(const string & strPost, const string key)
{
    unsigned char *pSrc = (unsigned char *) strPost.c_str();
    int iSrcLen = strPost.length();
    int iDestLen = iSrcLen * 2;
    unsigned char cDest[iDestLen] =
    { 0 };

    int writtenLen = 0;
//    mbedtls_base64_encode(cDest, iDestLen, &writtenLen, pSrc, iSrcLen);
//    base64_encode(cDest, &writtenLen, pSrc, iSrcLen);
    writtenLen = base64_encode(pSrc, iSrcLen, cDest, iDestLen);

    unsigned char output[20];

    unsigned char *pKey = (unsigned char *) key.data();
    int iKeyLen = key.length();

    sha1_hmac(pKey, iKeyLen, cDest, writtenLen, output);

    unsigned char md5Output[16];
//    mbedtls_md5(output, 20, md5Output);
    MD5(output, 20, md5Output);

    unsigned char result[32];

    unsigned char *pResult = result;
    for (int i = 0; i < 16; ++i)
    {
        sprintf((char*) pResult, "%02x", md5Output[i]);
        pResult += 2;
    }

    return (char *) result;
}

namespace allwinner
{

string PostMsgPackage::app_key = "5cbb8b70618e7bba8f560328821edbd1";
string PostMsgPackage::pack_id = ""; // = "com.aw.parrot.phone";
string PostMsgPackage::app_secret = "6cf8dae33a0c5f1f2c62dbb4aa10c580";
string PostMsgPackage::client_os_type = "4";
string PostMsgPackage::device_id = "12345678910";
string PostMsgPackage::access_token = "";
int PostMsgPackage::expires_in = 0;
int PostMsgPackage::gotTokenTime = 0;

PostMsgPackage::PostMsgPackage()
{
// TODO Auto-generated constructor stub
}

PostMsgPackage::~PostMsgPackage()
{
// TODO Auto-generated destructor stub
}

void PostMsgPackage::setAccessToken(const string& accessToken)
{
    if (access_token != accessToken)
    {
        gotTokenTime = time(0);
        access_token = accessToken;
    }
}

int PostMsgPackage::getExpiresIn() const
{
    return expires_in;
}

void PostMsgPackage::setExpiresIn(int expiresIn)
{
    expires_in = expiresIn;
}

void PostMsgPackage::setDeviceId(const string& deviceId)
{
    device_id = deviceId;
}

void PostMsgPackage::setAppKey(const string& appKey)
{
    app_key = appKey;
}

void PostMsgPackage::setAppSecret(const string& appSecret)
{
    app_secret = appSecret;
}

void PostMsgPackage::setClientOsType(const string& clientOsType)
{
    client_os_type = clientOsType;
}

void PostMsgPackage::setPackId(const string& packId)
{
    pack_id = packId;
}

bool PostMsgPackage::isExpires()
{
    if ((gotTokenTime + expires_in) <= time(0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

string PostMsgPackage::package(const string & strPost, bool isAccessToken)
{
    string result;

    map<string, string> myMap;
    if (isAccessToken)
    {
        stringstream ssNonce;
        stringstream ssTimestamp;
        unsigned int iTime = time(0);
        srand(iTime);
        int randNum = rand();
        ssNonce << randNum;
        string nonce;
        ssNonce >> nonce;
        ssTimestamp << iTime;
        string timestamp;
        ssTimestamp >> timestamp;

        myMap["grant_type"] = "client_credentials";
        myMap["nonce"] = nonce;
        myMap["timestamp"] = timestamp;

        myMap["client_id"] = app_key;
    }
    else
    {
        myMap["app_key"] = app_key;

        if (!access_token.empty())
        {
            myMap["access_token"] = access_token;
        }
    }
    myMap["client_os_type"] = client_os_type;

    if (pack_id != "")
    {
        myMap["pack_id"] = pack_id;
    }

    if (device_id != "")
    {
        myMap["device_id"] = device_id;
    }

    if (strPost != "")
    {
        vector<string> vectorPost;

        if (strPost.find("&") == string::npos)
        {
            vectorPost.push_back(strPost);
        }
        else
        {
            split(strPost, "&", vectorPost);
        }

        for (vector<string>::iterator it = vectorPost.begin(); it != vectorPost.end(); ++it)
        {
            vector<string> vKeyValuePair;
            split(*it, "=", vKeyValuePair);

            myMap[vKeyValuePair[0]] = vKeyValuePair[1];
        }
    }

    result = mapToPostStr(myMap);

    result = result + "&" + "sig=" + getSig(result, app_secret);
    cout << __func__ << "------result " << result << __LINE__ << endl;
    return result;
}

} /* namespace allwinner */
