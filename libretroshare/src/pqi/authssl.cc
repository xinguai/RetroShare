/*
 * libretroshare/src/pqi: authssl.cc
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2004-2008 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 *
 * This class is designed to provide authentication using ssl certificates
 * only. It is intended to be wrapped by an gpgauthmgr to provide
 * pgp + ssl web-of-trust authentication.
 *
 */

#include "authssl.h"
#include "sslfns.h"
#include "cleanupxpgp.h"

#include "pqinetwork.h"
#include "authgpg.h"
#include "pqi/p3connmgr.h"

/******************** notify of new Cert **************************/
#include "pqinotify.h"

#include <openssl/err.h>
//#include <openssl/evp.h>
//#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>

#include <sstream>
#include <iomanip>

/****
 * #define AUTHSSL_DEBUG 1
 ***/

/********************************************************************************/
/********************************************************************************/
/*********************   Cert Search / Add / Remove    **************************/
/********************************************************************************/
/********************************************************************************/

static int verify_x509_callback(int preverify_ok, X509_STORE_CTX *ctx);


// initialisation du pointeur de singleton à zéro
AuthSSL *AuthSSL::instance_ssl = new AuthSSL();


sslcert::sslcert(X509 *x509, std::string pid)
{
	certificate = x509;
	id = pid;
	name = getX509CNString(x509->cert_info->subject);
	org = getX509OrgString(x509->cert_info->subject);
	location = getX509LocString(x509->cert_info->subject);
	email = "";

	issuer = getX509CNString(x509->cert_info->issuer);

	authed = false;
}

/************************************************************************
 *
 *
 * CODE IS DIVIDED INTO
 *
 * 1) SSL Setup.
 * 3) Cert Access.
 * 4) Cert Sign / Verify.
 * 5) Cert Authentication
 * 2) Cert Add / Remove
 * 6) Cert Storage
 */

/********************************************************************************/
/********************************************************************************/
/*********************   Cert Search / Add / Remove    **************************/
/********************************************************************************/
/********************************************************************************/


AuthSSL::AuthSSL()
        : p3Config(CONFIG_TYPE_AUTHSSL), sslctx(NULL), 
	mOwnCert(NULL), mOwnPrivateKey(NULL), mOwnPublicKey(NULL), init(0)
{
}

bool AuthSSL::active()
{
	return init;
}


int	AuthSSL::InitAuth(const char *cert_file, const char *priv_key_file,
			const char *passwd)
{
#ifdef AUTHSSL_DEBUG
	std::cerr << "AuthSSL::InitAuth()";
	std::cerr << std::endl;
#endif

	/* single call here si don't need to invoke mutex yet */
static  int initLib = 0;
	if (!initLib)
	{
		initLib = 1;
		SSL_load_error_strings();
		SSL_library_init();
	}


	if (init == 1)
	{
                std::cerr << "AuthSSL::InitAuth already initialized." << std::endl;
		return 1;
	}

	if ((cert_file == NULL) ||
		(priv_key_file == NULL) ||
		(passwd == NULL))
	{
                //fprintf(stderr, "sslroot::initssl() missing parameters!\n");
		return 0;
	}


	// actions_to_seed_PRNG();
	RAND_seed(passwd, strlen(passwd));

	std::cerr << "SSL Library Init!" << std::endl;

	// setup connection method
	sslctx = SSL_CTX_new(TLSv1_method());

	// setup cipher lists.
	SSL_CTX_set_cipher_list(sslctx, "DEFAULT");

	// certificates (Set Local Server Certificate).
	FILE *ownfp = fopen(cert_file, "r");
	if (ownfp == NULL)
	{
		std::cerr << "Couldn't open Own Certificate!" << std::endl;
		return -1;
	}



	// get xPGP certificate.
	X509 *x509 = PEM_read_X509(ownfp, NULL, NULL, NULL);
	fclose(ownfp);

	if (x509 == NULL)
	{
                std::cerr << "AuthSSL::InitAuth() PEM_read_X509() Failed";
		std::cerr << std::endl;
		return -1;
	}
	SSL_CTX_use_certificate(sslctx, x509);
        mOwnPublicKey = X509_get_pubkey(x509);

	// get private key
	FILE *pkfp = fopen(priv_key_file, "rb");
	if (pkfp == NULL)
	{
		std::cerr << "Couldn't Open PrivKey File!" << std::endl;
		CloseAuth();
		return -1;
	}

        mOwnPrivateKey = PEM_read_PrivateKey(pkfp, NULL, NULL, (void *) passwd);
	fclose(pkfp);

        if (mOwnPrivateKey == NULL)
	{
                std::cerr << "AuthSSL::InitAuth() PEM_read_PrivateKey() Failed";
		std::cerr << std::endl;
		return -1;
	}
        SSL_CTX_use_PrivateKey(sslctx, mOwnPrivateKey);

	if (1 != SSL_CTX_check_private_key(sslctx))
	{
		std::cerr << "Issues With Private Key! - Doesn't match your Cert" << std::endl;
		std::cerr << "Check your input key/certificate:" << std::endl;
		std::cerr << priv_key_file << " & " << cert_file;
		std::cerr << std::endl;
		CloseAuth();
		return -1;
	}

	if (!getX509id(x509, mOwnId))
	{
		std::cerr << "AuthSSL::InitAuth() getX509id() Failed";
		std::cerr << std::endl;

		/* bad certificate */
		CloseAuth();
		return -1;
	}

	/* Check that Certificate is Ok ( virtual function )
	 * for gpg/pgp or CA verification
	 */

        if (!validateOwnCertificate(x509, mOwnPrivateKey))
	{
		std::cerr << "AuthSSL::InitAuth() validateOwnCertificate() Failed";
		std::cerr << std::endl;

		/* bad certificate */
		CloseAuth();
		exit(1);
		return -1;
	}


	// enable verification of certificates (PEER)
	// and install verify callback.
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER | 
			SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 
				verify_x509_callback);

	std::cerr << "SSL Verification Set" << std::endl;

	mOwnCert = new sslcert(x509, mOwnId);

	init = 1;
	return 1;
}

/* Dummy function to be overloaded by real implementation */
bool	AuthSSL::validateOwnCertificate(X509 *x509, EVP_PKEY *pkey)
{
	(void) pkey; /* remove unused parameter warning */

	/* standard authentication */
	if (!AuthX509WithGPG(x509))
	{
		return false;
	}
	return true;
}

bool	AuthSSL::CloseAuth()
{
#ifdef AUTHSSL_DEBUG
	std::cerr << "AuthSSL::CloseAuth()";
	std::cerr << std::endl;
#endif
	SSL_CTX_free(sslctx);

	// clean up private key....
	// remove certificates etc -> opposite of initssl.
	init = 0;
	return 1;
}

/* Context handling  */
SSL_CTX *AuthSSL::getCTX()
{
#ifdef AUTHSSL_DEBUG
	std::cerr << "AuthSSL::getCTX()";
	std::cerr << std::endl;
#endif
	return sslctx;
}

std::string AuthSSL::OwnId()
{
#ifdef AUTHSSL_DEBUG
//	std::cerr << "AuthSSL::OwnId()" << std::endl;
#endif
        return mOwnId;
}

std::string AuthSSL::getOwnLocation()
{
#ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::OwnId()" << std::endl;
#endif
        return mOwnCert->location;
}

std::string AuthSSL::SaveOwnCertificateToString()
{
#ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::SaveOwnCertificateToString() " << std::endl;
#endif
        return saveX509ToPEM(mOwnCert->certificate);
}

/********************************************************************************/
/********************************************************************************/
/*********************   Cert Search / Add / Remove    **************************/
/********************************************************************************/
/********************************************************************************/

bool AuthSSL::SignData(std::string input, std::string &sign)
{
	return SignData(input.c_str(), input.length(), sign);
}

bool AuthSSL::SignData(const void *data, const uint32_t len, std::string &sign)
{

	RsStackMutex stack(sslMtx);   /***** STACK LOCK MUTEX *****/

	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
        unsigned int signlen = EVP_PKEY_size(mOwnPrivateKey);
	unsigned char signature[signlen];

	if (0 == EVP_SignInit(mdctx, EVP_sha1()))
	{
		std::cerr << "EVP_SignInit Failure!" << std::endl;

		EVP_MD_CTX_destroy(mdctx);
		return false;
	}

	if (0 == EVP_SignUpdate(mdctx, data, len))
	{
		std::cerr << "EVP_SignUpdate Failure!" << std::endl;

		EVP_MD_CTX_destroy(mdctx);
		return false;
	}

        if (0 == EVP_SignFinal(mdctx, signature, &signlen, mOwnPrivateKey))
	{
		std::cerr << "EVP_SignFinal Failure!" << std::endl;

		EVP_MD_CTX_destroy(mdctx);
		return false;
	}

	EVP_MD_CTX_destroy(mdctx);

	sign.clear();	
	std::ostringstream out;
	out << std::hex;
	for(uint32_t i = 0; i < signlen; i++) 
	{
		out << std::setw(2) << std::setfill('0');
		out << (uint32_t) (signature[i]);
	}

	sign = out.str();

	return true;
}

bool AuthSSL::SignDataBin(std::string input, unsigned char *sign, unsigned int *signlen)
{
	return SignDataBin(input.c_str(), input.length(), sign, signlen);
}

bool AuthSSL::SignDataBin(const void *data, const uint32_t len, 
			unsigned char *sign, unsigned int *signlen)
{
	RsStackMutex stack(sslMtx);   /***** STACK LOCK MUTEX *****/
	return SSL_SignDataBin(data, len, sign, signlen, mOwnPrivateKey);
}


bool AuthSSL::VerifySignBin(const void *data, const uint32_t len,
                        unsigned char *sign, unsigned int signlen, SSL_id sslId)
{
	/* find certificate.
	 * if we don't have - fail.
         */

	RsStackMutex stack(sslMtx);   /***** STACK LOCK MUTEX *****/
	
	/* find the peer */
	sslcert *peer;
	if (sslId == mOwnId)
	{
		peer = mOwnCert;
	}
	else if (!locked_FindCert(sslId, &peer))
	{
		std::cerr << "VerifySignBin() no peer" << std::endl;
		return false;
	}

	return SSL_VerifySignBin(data, len, sign, signlen, peer->certificate);
}

bool AuthSSL::VerifyOwnSignBin(const void *data, const uint32_t len,
                        unsigned char *sign, unsigned int signlen) 
{
    return SSL_VerifySignBin(data, len, sign, signlen, mOwnCert->certificate);
}


/********************************************************************************/
/********************************************************************************/
/*********************   Sign and Auth with GPG        **************************/
/********************************************************************************/
/********************************************************************************/

/* Note these functions don't need Mutexes - 
 * only using GPG functions - which lock themselves
 */

X509 *AuthSSL::SignX509ReqWithGPG(X509_REQ *req, long days)
{
        /* Transform the X509_REQ into a suitable format to
         * generate DIGEST hash. (for SSL to do grunt work)
         */

#define SERIAL_RAND_BITS 64

        //const EVP_MD *digest = EVP_sha1();
        ASN1_INTEGER *serial = ASN1_INTEGER_new();
        EVP_PKEY *tmppkey;
        X509 *x509 = X509_new();
        if (x509 == NULL)
        {
                std::cerr << "AuthSSL::SignX509Req() FAIL" << std::endl;
                return NULL;
        }

        //long version = 0x00;
        unsigned long chtype = MBSTRING_ASC;
        X509_NAME *issuer_name = X509_NAME_new();
        X509_NAME_add_entry_by_txt(issuer_name, "CN", chtype,
                        (unsigned char *) AuthGPG::getAuthGPG()->getGPGOwnId().c_str(), -1, -1, 0);
/****
        X509_NAME_add_entry_by_NID(issuer_name, 48, 0,
                        (unsigned char *) "email@email.com", -1, -1, 0);
        X509_NAME_add_entry_by_txt(issuer_name, "O", chtype,
                        (unsigned char *) "org", -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509_name, "L", chtype,
                        (unsigned char *) "loc", -1, -1, 0);
****/

        std::cerr << "AuthSSL::SignX509Req() Issuer name: " << AuthGPG::getAuthGPG()->getGPGOwnId() << std::endl;

        BIGNUM *btmp = BN_new();
        if (!BN_pseudo_rand(btmp, SERIAL_RAND_BITS, 0, 0))
        {
                std::cerr << "AuthSSL::SignX509Req() rand FAIL" << std::endl;
                return NULL;
        }
        if (!BN_to_ASN1_INTEGER(btmp, serial))
        {
                std::cerr << "AuthSSL::SignX509Req() asn1 FAIL" << std::endl;
                return NULL;
        }
        BN_free(btmp);

        if (!X509_set_serialNumber(x509, serial))
        {
                std::cerr << "AuthSSL::SignX509Req() serial FAIL" << std::endl;
                return NULL;
        }
        ASN1_INTEGER_free(serial);

        /* Generate SUITABLE issuer name.
         * Must reference OpenPGP key, that is used to verify it
         */

        if (!X509_set_issuer_name(x509, issuer_name))
        {
                std::cerr << "AuthSSL::SignX509Req() issue FAIL" << std::endl;
                return NULL;
        }
        X509_NAME_free(issuer_name);


        if (!X509_gmtime_adj(X509_get_notBefore(x509),0))
        {
                std::cerr << "AuthSSL::SignX509Req() notbefore FAIL" << std::endl;
                return NULL;
        }

        if (!X509_gmtime_adj(X509_get_notAfter(x509), (long)60*60*24*days))
        {
                std::cerr << "AuthSSL::SignX509Req() notafter FAIL" << std::endl;
                return NULL;
        }

        if (!X509_set_subject_name(x509, X509_REQ_get_subject_name(req)))
        {
                std::cerr << "AuthSSL::SignX509Req() sub FAIL" << std::endl;
                return NULL;
        }

        tmppkey = X509_REQ_get_pubkey(req);
        if (!tmppkey || !X509_set_pubkey(x509,tmppkey))
        {
                std::cerr << "AuthSSL::SignX509Req() pub FAIL" << std::endl;
                return NULL;
        }

        std::cerr << "X509 Cert, prepared for signing" << std::endl;

        /*** NOW The Manual signing bit (HACKED FROM asn1/a_sign.c) ***/
        int (*i2d)(X509_CINF*, unsigned char**) = i2d_X509_CINF;
        X509_ALGOR *algor1 = x509->cert_info->signature;
        X509_ALGOR *algor2 = x509->sig_alg;
        ASN1_BIT_STRING *signature = x509->signature;
        X509_CINF *data = x509->cert_info;
        //EVP_PKEY *pkey = NULL;
        const EVP_MD *type = EVP_sha1();

        EVP_MD_CTX ctx;
        unsigned char *p,*buf_in=NULL;
        unsigned char *buf_hashout=NULL,*buf_sigout=NULL;
        int inl=0,hashoutl=0,hashoutll=0;
        int sigoutl=0,sigoutll=0;
        X509_ALGOR *a;

        EVP_MD_CTX_init(&ctx);

        /* FIX ALGORITHMS */

        a = algor1;
        ASN1_TYPE_free(a->parameter);
        a->parameter=ASN1_TYPE_new();
        a->parameter->type=V_ASN1_NULL;

        ASN1_OBJECT_free(a->algorithm);
        a->algorithm=OBJ_nid2obj(type->pkey_type);

        a = algor2;
        ASN1_TYPE_free(a->parameter);
        a->parameter=ASN1_TYPE_new();
        a->parameter->type=V_ASN1_NULL;

        ASN1_OBJECT_free(a->algorithm);
        a->algorithm=OBJ_nid2obj(type->pkey_type);


        std::cerr << "Algorithms Fixed" << std::endl;

        /* input buffer */
        inl=i2d(data,NULL);
        buf_in=(unsigned char *)OPENSSL_malloc((unsigned int)inl);

        hashoutll=hashoutl=EVP_MD_size(type);
        buf_hashout=(unsigned char *)OPENSSL_malloc((unsigned int)hashoutl);

        sigoutll=sigoutl=2048; // hashoutl; //EVP_PKEY_size(pkey);
        buf_sigout=(unsigned char *)OPENSSL_malloc((unsigned int)sigoutl);

        if ((buf_in == NULL) || (buf_hashout == NULL) || (buf_sigout == NULL))
                {
                hashoutl=0;
                sigoutl=0;
                fprintf(stderr, "AuthSSL::SignX509Req: ASN1err(ASN1_F_ASN1_SIGN,ERR_R_MALLOC_FAILURE)\n");
                goto err;
                }
        p=buf_in;

        std::cerr << "Buffers Allocated" << std::endl;

        i2d(data,&p);
        /* data in buf_in, ready to be hashed */
        EVP_DigestInit_ex(&ctx,type, NULL);
        EVP_DigestUpdate(&ctx,(unsigned char *)buf_in,inl);
        if (!EVP_DigestFinal(&ctx,(unsigned char *)buf_hashout,
                        (unsigned int *)&hashoutl))
                {
                hashoutl=0;
                fprintf(stderr, "AuthSSL::SignX509Req: ASN1err(ASN1_F_ASN1_SIGN,ERR_R_EVP_LIB)\n");
                goto err;
                }

        std::cerr << "Digest Applied: len: " << hashoutl << std::endl;

        /* NOW Sign via GPG Functions */
        if (!AuthGPG::getAuthGPG()->SignDataBin(buf_hashout, hashoutl, buf_sigout, (unsigned int *) &sigoutl))
        {
                sigoutl = 0;
                goto err;
        }

        std::cerr << "Buffer Sizes: in: " << inl;
        std::cerr << "  HashOut: " << hashoutl;
        std::cerr << "  SigOut: " << sigoutl;
        std::cerr << std::endl;

        //passphrase = "NULL";

        std::cerr << "Signature done: len:" << sigoutl << std::endl;

        /* ADD Signature back into Cert... Signed!. */

        if (signature->data != NULL) OPENSSL_free(signature->data);
        signature->data=buf_sigout;
        buf_sigout=NULL;
        signature->length=sigoutl;
        /* In the interests of compatibility, I'll make sure that
         * the bit string has a 'not-used bits' value of 0
         */
        signature->flags&= ~(ASN1_STRING_FLAG_BITS_LEFT|0x07);
        signature->flags|=ASN1_STRING_FLAG_BITS_LEFT;

        std::cerr << "Certificate Complete" << std::endl;

        return x509;

	/* XXX CLEANUP */
  err:
        /* cleanup */
        std::cerr << "GPGAuthMgr::SignX509Req() err: FAIL" << std::endl;

        return NULL;
}


/* This function, checks that the X509 is signed by a known GPG key,
 * NB: we do not have to have approved this person as a friend.
 * this is important - as it allows non-friends messages to be validated.
 */

bool AuthSSL::AuthX509WithGPG(X509 *x509)
{
        #ifdef AUTHSSL_DEBUG
        fprintf(stderr, "AuthSSL::AuthX509WithGPG() called\n");
        #endif

        if (!CheckX509Certificate(x509))
	{
            std::cerr << "AuthSSL::AuthX509() X509 NOT authenticated : Certificate failed basic checks" << std::endl;
            return false;
        }

        /* extract CN for peer Id */
        std::string issuer = getX509CNString(x509->cert_info->issuer);
        RsPeerDetails pd;
        #ifdef AUTHSSL_DEBUG
        std::cerr << "Checking GPG issuer : " << issuer << std::endl ;
        #endif
        if (!AuthGPG::getAuthGPG()->getGPGDetails(issuer, pd)) {
            std::cerr << "AuthSSL::AuthX509() X509 NOT authenticated : AuthGPG::getAuthGPG()->getGPGDetails() returned false." << std::endl;
            return false;
        }

        /* verify GPG signature */

        /*** NOW The Manual signing bit (HACKED FROM asn1/a_sign.c) ***/
        int (*i2d)(X509_CINF*, unsigned char**) = i2d_X509_CINF;
        ASN1_BIT_STRING *signature = x509->signature;
        X509_CINF *data = x509->cert_info;
        const EVP_MD *type = EVP_sha1();

        EVP_MD_CTX ctx;
        unsigned char *p,*buf_in=NULL;
        unsigned char *buf_hashout=NULL,*buf_sigout=NULL;
        int inl=0,hashoutl=0,hashoutll=0;
        int sigoutl=0,sigoutll=0;
        //X509_ALGOR *a;

        EVP_MD_CTX_init(&ctx);

        /* input buffer */
        inl=i2d(data,NULL);
        buf_in=(unsigned char *)OPENSSL_malloc((unsigned int)inl);

        hashoutll=hashoutl=EVP_MD_size(type);
        buf_hashout=(unsigned char *)OPENSSL_malloc((unsigned int)hashoutl);

        sigoutll=sigoutl=2048; //hashoutl; //EVP_PKEY_size(pkey);
        buf_sigout=(unsigned char *)OPENSSL_malloc((unsigned int)sigoutl);

        #ifdef AUTHSSL_DEBUG
        std::cerr << "Buffer Sizes: in: " << inl;
        std::cerr << "  HashOut: " << hashoutl;
        std::cerr << "  SigOut: " << sigoutl;
        std::cerr << std::endl;
        #endif

        if ((buf_in == NULL) || (buf_hashout == NULL) || (buf_sigout == NULL)) {
                hashoutl=0;
                sigoutl=0;
                fprintf(stderr, "AuthSSL::AuthX509: ASN1err(ASN1_F_ASN1_SIGN,ERR_R_MALLOC_FAILURE)\n");
                goto err;
        }
        p=buf_in;

        #ifdef AUTHSSL_DEBUG
        std::cerr << "Buffers Allocated" << std::endl;
        #endif

        i2d(data,&p);
        /* data in buf_in, ready to be hashed */
        EVP_DigestInit_ex(&ctx,type, NULL);
        EVP_DigestUpdate(&ctx,(unsigned char *)buf_in,inl);
        if (!EVP_DigestFinal(&ctx,(unsigned char *)buf_hashout,
                        (unsigned int *)&hashoutl))
                {
                hashoutl=0;
                fprintf(stderr, "AuthSSL::AuthX509: ASN1err(ASN1_F_ASN1_SIGN,ERR_R_EVP_LIB)\n");
                goto err;
                }

        #ifdef AUTHSSL_DEBUG
        std::cerr << "Digest Applied: len: " << hashoutl << std::endl;
        #endif

        /* copy data into signature */
        sigoutl = signature->length;
        memmove(buf_sigout, signature->data, sigoutl);

        /* NOW check sign via GPG Functions */
        //get the fingerprint of the key that is supposed to sign
        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::AuthX509() verifying the gpg sig with keyprint : " << pd.fpr << std::endl;
		  std::cerr << "Sigoutl = " << sigoutl << std::endl ;
		  std::cerr << "pd.fpr = " << pd.fpr << std::endl ;
		  std::cerr << "hashoutl = " << hashoutl << std::endl ;
        #endif

        if (!AuthGPG::getAuthGPG()->VerifySignBin(buf_hashout, hashoutl, buf_sigout, (unsigned int) sigoutl, pd.fpr)) {
                sigoutl = 0;
                goto err;
        }

        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::AuthX509() X509 authenticated" << std::endl;
        #endif

        return true;

  err:
        std::cerr << "AuthSSL::AuthX509() X509 NOT authenticated" << std::endl;
        return false;
}



	/* validate + get id */
bool    AuthSSL::ValidateCertificate(X509 *x509, std::string &peerId)
{
	/* check self signed */
	if (!AuthX509WithGPG(x509))
	{
#ifdef AUTHSSL_DEBUG
		std::cerr << "AuthSSL::ValidateCertificate() bad certificate.";
		std::cerr << std::endl;
#endif
		return false;
	}
	if(!getX509id(x509, peerId)) 
	{
#ifdef AUTHSSL_DEBUG
		std::cerr << "AuthSSL::ValidateCertificate() Cannot retrieve peer id from certificate..";
		std::cerr << std::endl;
#endif
		return false;
	}

#ifdef AUTHSSL_DEBUG
	std::cerr << "AuthSSL::ValidateCertificate() good certificate.";
	std::cerr << std::endl;
#endif

	return true;
}


/********************************************************************************/
/********************************************************************************/
/****************************  encrypt / decrypt fns ****************************/
/********************************************************************************/
/********************************************************************************/

static int verify_x509_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
#ifdef AUTHSSL_DEBUG
        std::cerr << "static verify_x509_callback called.";
        std::cerr << std::endl;
#endif
        return AuthSSL::getAuthSSL()->VerifyX509Callback(preverify_ok, ctx);

}

int AuthSSL::VerifyX509Callback(int preverify_ok, X509_STORE_CTX *ctx)
{
        char    buf[256];
        X509   *err_cert;
        int     err, depth;

        err_cert = X509_STORE_CTX_get_current_cert(ctx);
        err = X509_STORE_CTX_get_error(ctx);
        depth = X509_STORE_CTX_get_error_depth(ctx);

        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::VerifyX509Callback(preverify_ok: " << preverify_ok
                                 << " Err: " << err << " Depth: " << depth << std::endl;
        #endif

        /*
        * Retrieve the pointer to the SSL of the connection currently treated
        * and the application specific data stored into the SSL object.
        */

        X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::VerifyX509Callback: depth: " << depth << ":" << buf << std::endl;
        #endif


        if (!preverify_ok) {
                #ifdef AUTHSSL_DEBUG
                fprintf(stderr, "Verify error:num=%d:%s:depth=%d:%s\n", err,
                X509_verify_cert_error_string(err), depth, buf);
                #endif
        }

        /*
        * At this point, err contains the last verification error. We can use
        * it for something special
        */

        if (!preverify_ok)
        {

            X509_NAME_oneline(X509_get_issuer_name(X509_STORE_CTX_get_current_cert(ctx)), buf, 256);
            #ifdef AUTHSSL_DEBUG
            printf("issuer= %s\n", buf);
            #endif

            #ifdef AUTHSSL_DEBUG
            fprintf(stderr, "Doing REAL PGP Certificates\n");
            #endif
            /* do the REAL Authentication */
            if (!AuthX509WithGPG(X509_STORE_CTX_get_current_cert(ctx)))
            {
                    #ifdef AUTHSSL_DEBUG
                    fprintf(stderr, "AuthSSL::VerifyX509Callback() X509 not authenticated.\n");
                    #endif
                    return false;
            }
            std::string pgpid = getX509CNString(X509_STORE_CTX_get_current_cert(ctx)->cert_info->issuer);
            if (!AuthGPG::getAuthGPG()->isGPGAccepted(pgpid))
            {
                    #ifdef AUTHSSL_DEBUG
                    fprintf(stderr, "AuthSSL::VerifyX509Callback() pgp key not accepted : \n");
                    fprintf(stderr, "issuer pgpid : ");
                    fprintf(stderr, "%s\n",pgpid.c_str());
                    fprintf(stderr, "\n AuthGPG::getAuthGPG()->getGPGOwnId() : ");
                    fprintf(stderr, "%s\n",AuthGPG::getAuthGPG()->getGPGOwnId().c_str());
                    fprintf(stderr, "\n");
                    #endif
                    return false;
            }

            preverify_ok = true;

        } else {
                #ifdef AUTHSSL_DEBUG
                fprintf(stderr, "A normal certificate is probably a security breach attempt. We sould fail it !!!\n");
                #endif
                preverify_ok = false;
        }

        if (preverify_ok) {

            //sslcert *cert = NULL;
            std::string certId;
            getX509id(X509_STORE_CTX_get_current_cert(ctx), certId);

        }

        #ifdef AUTHSSL_DEBUG
        if (preverify_ok) {
            fprintf(stderr, "AuthSSL::VerifyX509Callback returned true.\n");
        } else {
            fprintf(stderr, "AuthSSL::VerifyX509Callback returned false.\n");
        }
        #endif

        return preverify_ok;
}


/********************************************************************************/
/********************************************************************************/
/****************************  encrypt / decrypt fns ****************************/
/********************************************************************************/
/********************************************************************************/


bool    AuthSSL::encrypt(void *&out, int &outlen, const void *in, int inlen, std::string peerId)
{
	RsStackMutex stack(sslMtx); /******* LOCKED ******/

#ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::encrypt() called for peerId : " << peerId << " with inlen : " << inlen << std::endl;
#endif
        //TODO : use ssl to crypt the binary input buffer
//        out = malloc(inlen);
//        memcpy(out, in, inlen);
//        outlen = inlen;

        EVP_PKEY *public_key;
        if (peerId == mOwnId) {
            public_key = mOwnPublicKey;
        } else {
            if (!mCerts[peerId]) {
                #ifdef AUTHSSL_DEBUG
                std::cerr << "AuthSSL::encrypt() public key not found." << std::endl;
                #endif
                return false;
            } else {
                public_key = mCerts[peerId]->certificate->cert_info->key->pkey;
            }
        }

        int out_offset = 0;
        out = malloc(inlen + 2048);

        /// ** from demos/maurice/example1.c of openssl V1.0 *** ///
        unsigned char * iv = new unsigned char [16];
        memset(iv, '\0', 16);
        unsigned char * ek = new unsigned char [EVP_PKEY_size(public_key) + 1024];
        uint32_t ekl, net_ekl;
        unsigned char * cryptBuff = new unsigned char [inlen + 16];
        memset(cryptBuff, '\0', sizeof(cryptBuff));
        int cryptBuffL = 0;
        unsigned char key[256];

        /// ** copied implementation of EVP_SealInit of openssl V1.0 *** ///;
        EVP_CIPHER_CTX cipher_ctx;
        EVP_CIPHER_CTX_init(&cipher_ctx);

        if(!EVP_EncryptInit_ex(&cipher_ctx,EVP_aes_256_cbc(),NULL,NULL,NULL)) {
            return false;
        }

        if (EVP_CIPHER_CTX_rand_key(&cipher_ctx, key) <= 0) {
            return false;
        }

        if (EVP_CIPHER_CTX_iv_length(&cipher_ctx)) {
            RAND_pseudo_bytes(iv,EVP_CIPHER_CTX_iv_length(&cipher_ctx));
        }

        if(!EVP_EncryptInit_ex(&cipher_ctx,NULL,NULL,key,iv)) {
            return false;
        }

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
        ekl=EVP_PKEY_encrypt_old(ek,key,EVP_CIPHER_CTX_key_length(&cipher_ctx), public_key);
#else
        ekl=EVP_PKEY_encrypt(ek,key,EVP_CIPHER_CTX_key_length(&cipher_ctx), public_key);
#endif

        /// ** copied implementation of EVP_SealInit of openssl V *** ///

        net_ekl = htonl(ekl);
        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), (char*)&net_ekl, sizeof(net_ekl));
        out_offset += sizeof(net_ekl);

        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), ek, ekl);
        out_offset += ekl;

        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), iv, 16);
        out_offset += 16;

        EVP_EncryptUpdate(&cipher_ctx, cryptBuff, &cryptBuffL, (unsigned char*)in, inlen);
        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), cryptBuff, cryptBuffL);
        out_offset += cryptBuffL;

        EVP_EncryptFinal_ex(&cipher_ctx, cryptBuff, &cryptBuffL);
        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), cryptBuff, cryptBuffL);
        out_offset += cryptBuffL;

        outlen = out_offset;

        EVP_EncryptInit_ex(&cipher_ctx,NULL,NULL,NULL,NULL);
        EVP_CIPHER_CTX_cleanup(&cipher_ctx);


        delete[] ek;

        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::encrypt() finished with outlen : " << outlen << std::endl;
        #endif

        //free(ek);
        //free(cryptBuff);
        //free(iv);

        return true;
}

bool    AuthSSL::decrypt(void *&out, int &outlen, const void *in, int inlen)
{
	RsStackMutex stack(sslMtx); /******* LOCKED ******/



#ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::decrypt() called with inlen : " << inlen << std::endl;
#endif
        //TODO : use ssl to decrypt the binary input buffer
//        out = malloc(inlen);
//        memcpy(out, in, inlen);
//        outlen = inlen;
        out = malloc(inlen + 16);
        int in_offset = 0;
        unsigned char * buf = new unsigned char [inlen + 16];
        memset(buf, '\0', sizeof(buf));
        int buflen = 0;
        EVP_CIPHER_CTX ectx;
        unsigned char * iv = new unsigned char [16];
        memset(iv, '\0', 16);
        unsigned char *encryptKey;
        unsigned int ekeylen;


        memcpy(&ekeylen, (void*)((unsigned long int)in + (unsigned long int)in_offset), sizeof(ekeylen));
        in_offset += sizeof(ekeylen);

        ekeylen = ntohl(ekeylen);

        if (ekeylen != (unsigned) EVP_PKEY_size(mOwnPrivateKey))
        {
                fprintf(stderr, "keylength mismatch");
                return false;
        }

        encryptKey = new unsigned char [sizeof(char) * ekeylen];

        memcpy(encryptKey, (void*)((unsigned long int)in + (unsigned long int)in_offset), ekeylen);
        in_offset += ekeylen;

        memcpy(iv, (void*)((unsigned long int)in + (unsigned long int)in_offset), 16);
        in_offset += 16;

//        EVP_OpenInit(&ectx,
//                   EVP_des_ede3_cbc(),
//                   encryptKey,
//                   ekeylen,
//                   iv,
//                   privateKey);
        /// ** copied implementation of EVP_SealInit of openssl V1.0 *** ///;

        unsigned char *key=NULL;
        int i=0;

        EVP_CIPHER_CTX_init(&ectx);
        if(!EVP_DecryptInit_ex(&ectx,EVP_aes_256_cbc(),NULL, NULL,NULL)) return false;

        if (mOwnPrivateKey->type != EVP_PKEY_RSA)
        {
            return false;
        }

        key=(unsigned char *)OPENSSL_malloc(256);
        if (key == NULL)
        {
            return false;
        }

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
        i=EVP_PKEY_decrypt_old(key,encryptKey,ekeylen,mOwnPrivateKey);
#else
        i=EVP_PKEY_decrypt(key,encryptKey,ekeylen,mOwnPrivateKey);
#endif
        if ((i <= 0) || !EVP_CIPHER_CTX_set_key_length(&ectx, i))
        {
            return false;
        }

        if(!EVP_DecryptInit_ex(&ectx,NULL,NULL,key,iv)) return false;
        /// ** copied implementation of EVP_SealInit of openssl V1.0 *** ///;


        if (!EVP_DecryptUpdate(&ectx, buf, &buflen, (unsigned char*)((unsigned long int)in + (unsigned long int)in_offset), inlen - in_offset)) {
            return false;
        }
        memcpy(out, buf, buflen);
        int out_offset = buflen;

        if (!EVP_DecryptFinal(&ectx, buf, &buflen)) {
            return false;
        }
        memcpy((void*)((unsigned long int)out + (unsigned long int)out_offset), buf, buflen);
        out_offset += buflen;
        outlen = out_offset;

        EVP_DecryptInit_ex(&ectx,NULL,NULL, NULL,NULL);
        EVP_CIPHER_CTX_cleanup(&ectx);

        delete[] encryptKey;

        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::decrypt() finished with outlen : " << outlen << std::endl;
        #endif

        return true;
}


/********************************************************************************/
/********************************************************************************/
/*********************   Cert Search / Add / Remove    **************************/
/********************************************************************************/
/********************************************************************************/

/* store for discovery */
bool    AuthSSL::FailedCertificate(X509 *x509, bool incoming)
{
	(void) incoming; /* remove unused parameter warning */

	/* if auths -> store */
	if (AuthX509WithGPG(x509))
	{
		LocalStoreCert(x509);
		return true;
	}
	return false;
}

bool    AuthSSL::CheckCertificate(std::string id, X509 *x509)
{
	(void) id; /* remove unused parameter warning */

	/* if auths -> store */
	if (AuthX509WithGPG(x509))
	{
		LocalStoreCert(x509);
		return true;
	}
	return false;
}



/* Locked search -> internal help function */
bool AuthSSL::locked_FindCert(std::string id, sslcert **cert)
{
	std::map<std::string, sslcert *>::iterator it;
	
	if (mCerts.end() != (it = mCerts.find(id)))
	{
		*cert = it->second;
		return true;
	}
	return false;
}


/* Remove Certificate */

bool AuthSSL::RemoveX509(std::string id)
{
	std::map<std::string, sslcert *>::iterator it;
	
	RsStackMutex stack(sslMtx); /******* LOCKED ******/

	if (mCerts.end() != (it = mCerts.find(id)))
	{
		sslcert *cert = it->second;

		/* clean up */
		X509_free(cert->certificate);
		cert->certificate = NULL;
		delete cert;

		mCerts.erase(it);

		return true;
	}
	return false;
}


bool AuthSSL::LocalStoreCert(X509* x509) 
{
	//store the certificate in the local cert list
	std::string peerId;
	if(!getX509id(x509, peerId))
	{
		std::cerr << "AuthSSL::LocalStoreCert() Cannot retrieve peer id from certificate." << std::endl;
#ifdef AUTHSSL_DEBUG
#endif
		return false;
	}


	RsStackMutex stack(sslMtx); /******* LOCKED ******/

	if (peerId == mOwnId) 
	{
#ifdef AUTHSSL_DEBUG
		std::cerr << "AuthSSL::LocalStoreCert() not storing own certificate" << std::endl;
#endif
		return false;
	}

	/* do a search */
	std::map<std::string, sslcert *>::iterator it;
	
	if (mCerts.end() != (it = mCerts.find(peerId)))
	{
		sslcert *cert = it->second;

		/* found something */
		/* check that they are exact */
		if (0 != X509_cmp(cert->certificate, x509))
		{
			/* MAJOR ERROR */
			std::cerr << "ERROR : AuthSSL::LocalStoreCert() got two ssl certificates with identical ids -> dropping second";
			std::cerr << std::endl;
			return false;
		}
		/* otherwise - we have it already */
		return false;
	}

#ifdef AUTHSSL_DEBUG
	std::cerr << "AuthSSL::LocalStoreCert() storing certificate for " << peerId << std::endl;
#endif
	mCerts[peerId] = new sslcert(X509_dup(x509), peerId);

	/* flag for saving config */
	IndicateConfigChanged();
	return true ;
}


/********************************************************************************/
/********************************************************************************/
/************************   Config Functions   **********************************/
/********************************************************************************/
/********************************************************************************/


RsSerialiser *AuthSSL::setupSerialiser()
{
        RsSerialiser *rss = new RsSerialiser ;
        rss->addSerialType(new RsGeneralConfigSerialiser());
        return rss ;
}

std::list<RsItem*> AuthSSL::saveList(bool& cleanup)
{
        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::saveList() called" << std::endl ;
        #endif

        RsStackMutex stack(sslMtx); /******* LOCKED ******/

        cleanup = true ;
        std::list<RsItem*> lst ;


        // Now save config for network digging strategies
        RsConfigKeyValueSet *vitem = new RsConfigKeyValueSet ;
        std::map<std::string, sslcert*>::iterator mapIt;
        for (mapIt = mCerts.begin(); mapIt != mCerts.end(); mapIt++) {
            if (mapIt->first == mOwnId) {
                continue;
            }
            RsTlvKeyValue kv;
            kv.key = mapIt->first;
            #ifdef AUTHSSL_DEBUG
            std::cerr << "AuthSSL::saveList() called (mapIt->first) : " << (mapIt->first) << std::endl ;
            #endif
            kv.value = saveX509ToPEM(mapIt->second->certificate);
            vitem->tlvkvs.pairs.push_back(kv) ;
        }
        lst.push_back(vitem);

        return lst ;
}

bool AuthSSL::loadList(std::list<RsItem*> load)
{
        #ifdef AUTHSSL_DEBUG
        std::cerr << "AuthSSL::loadList() Item Count: " << load.size() << std::endl;
        #endif

        /* load the list of accepted gpg keys */
        std::list<RsItem *>::iterator it;
        for(it = load.begin(); it != load.end(); it++) {
                RsConfigKeyValueSet *vitem = dynamic_cast<RsConfigKeyValueSet *>(*it);

                if(vitem) {
                        #ifdef AUTHSSL_DEBUG
                        std::cerr << "AuthSSL::loadList() General Variable Config Item:" << std::endl;
                        vitem->print(std::cerr, 10);
                        std::cerr << std::endl;
                        #endif

                        std::list<RsTlvKeyValue>::iterator kit;
                        for(kit = vitem->tlvkvs.pairs.begin(); kit != vitem->tlvkvs.pairs.end(); kit++) {
                            if (kit->key == mOwnId) {
                                continue;
                            }

                            X509 *peer = loadX509FromPEM(kit->value);
			    /* authenticate it */
			    if (AuthX509WithGPG(peer))
			    {
				LocalStoreCert(peer);
			    }
                        }
                }
                delete (*it);
        }
        return true;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

