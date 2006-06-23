/*************************************************
* PKCS #10/Self Signed Cert Creation Source File *
* (C) 1999-2006 The Botan Project                *
*************************************************/

#include <botan/x509self.h>
#include <botan/x509_ext.h>
#include <botan/x509_ca.h>
#include <botan/der_enc.h>
#include <botan/conf.h>
#include <botan/look_pk.h>
#include <botan/oids.h>
#include <botan/pipe.h>
#include <memory>

namespace Botan {

namespace {

/*************************************************
* Shared setup for self-signed items             *
*************************************************/
MemoryVector<byte> shared_setup(const X509_Cert_Options& opts,
                                const PKCS8_PrivateKey& key)
   {
   const PKCS8_PrivateKey* key_pointer = &key;
   if(!dynamic_cast<const PK_Signing_Key*>(key_pointer))
      throw Invalid_Argument("Key type " + key.algo_name() + " cannot sign");

   opts.sanity_check();

   Pipe key_encoder;
   key_encoder.start_msg();
   X509::encode(key, key_encoder, RAW_BER);
   key_encoder.end_msg();

   return key_encoder.read_all();
   }

/*************************************************
* Load information from the X509_Cert_Options    *
*************************************************/
void load_info(const X509_Cert_Options& opts, X509_DN& subject_dn,
               AlternativeName& subject_alt)
   {
   subject_dn.add_attribute("X520.CommonName", opts.common_name);
   subject_dn.add_attribute("X520.Country", opts.country);
   subject_dn.add_attribute("X520.State", opts.state);
   subject_dn.add_attribute("X520.Locality", opts.locality);
   subject_dn.add_attribute("X520.Organization", opts.organization);
   subject_dn.add_attribute("X520.OrganizationalUnit", opts.org_unit);
   subject_dn.add_attribute("X520.SerialNumber", opts.serial_number);
   subject_alt = AlternativeName(opts.email, opts.uri, opts.dns);
   subject_alt.add_othername(OIDS::lookup("PKIX.XMPPAddr"),
                             opts.xmpp, UTF8_STRING);
   }

/*************************************************
* Choose a signing format for the key            *
*************************************************/
PK_Signer* choose_sig_format(const PKCS8_PrivateKey& key,
                             AlgorithmIdentifier& sig_algo)
   {
   std::string padding;
   Signature_Format format;
   Config::choose_sig_format(key.algo_name(), padding, format);

   sig_algo.oid = OIDS::lookup(key.algo_name() + "/" + padding);
   sig_algo.parameters = key.DER_encode_params();

   const PK_Signing_Key& sig_key = dynamic_cast<const PK_Signing_Key&>(key);

   return get_pk_signer(sig_key, padding, format);
   }

}

namespace X509 {

/*************************************************
* Create a new self-signed X.509 certificate     *
*************************************************/
X509_Certificate create_self_signed_cert(const X509_Cert_Options& opts,
                                         const PKCS8_PrivateKey& key)
   {
   AlgorithmIdentifier sig_algo;
   X509_DN subject_dn;
   AlternativeName subject_alt;

   MemoryVector<byte> pub_key = shared_setup(opts, key);
   std::auto_ptr<PK_Signer> signer(choose_sig_format(key, sig_algo));
   load_info(opts, subject_dn, subject_alt);

   Key_Constraints constraints;
   if(opts.is_CA)
      constraints = Key_Constraints(KEY_CERT_SIGN | CRL_SIGN);
   else
      constraints = find_constraints(key, opts.constraints);

   return X509_CA::make_cert(signer.get(), sig_algo, pub_key,
                             MemoryVector<byte>(), opts.start, opts.end,
                             subject_dn, subject_dn,
                             opts.is_CA, opts.path_limit,
                             subject_alt, subject_alt,
                             constraints, opts.ex_constraints);
   }

/*************************************************
* Create a PKCS #10 certificate request          *
*************************************************/
PKCS10_Request create_cert_req(const X509_Cert_Options& opts,
                               const PKCS8_PrivateKey& key)
   {
   AlgorithmIdentifier sig_algo;
   X509_DN subject_dn;
   AlternativeName subject_alt;

   MemoryVector<byte> pub_key = shared_setup(opts, key);
   std::auto_ptr<PK_Signer> signer(choose_sig_format(key, sig_algo));
   load_info(opts, subject_dn, subject_alt);

   const u32bit PKCS10_VERSION = 0;

   Extensions extensions;

   extensions.add(
      new Cert_Extension::Basic_Constraints(opts.is_CA, opts.path_limit));
   extensions.add(
      new Cert_Extension::Key_Usage(
         opts.is_CA ? Key_Constraints(KEY_CERT_SIGN | CRL_SIGN) :
                      find_constraints(key, opts.constraints)
         )
      );
   extensions.add(
      new Cert_Extension::Extended_Key_Usage(opts.ex_constraints));
   extensions.add(
      new Cert_Extension::Subject_Alternative_Name(subject_alt));

   DER_Encoder tbs_req;

   tbs_req.start_cons(SEQUENCE)
      .encode(PKCS10_VERSION)
      .encode(subject_dn)
      .raw_bytes(pub_key)
      .start_explicit(0);

   if(opts.challenge != "")
      {
      ASN1_String challenge(opts.challenge, DIRECTORY_STRING);

      tbs_req.encode(
         Attribute("PKCS9.ChallengePassword",
                   DER_Encoder().encode(challenge).get_contents()
            )
         );
      }

   tbs_req.encode(
      Attribute("PKCS9.ExtensionRequest",
                DER_Encoder()
                   .start_cons(SEQUENCE)
                      .encode(extensions)
                   .end_cons()
               .get_contents()
         )
      )
      .end_explicit()
      .end_cons();

   MemoryVector<byte> tbs_bits = tbs_req.get_contents();
   MemoryVector<byte> sig = signer->sign_message(tbs_bits);

   DataSource_Memory source(
      DER_Encoder()
         .start_cons(SEQUENCE)
            .raw_bytes(tbs_bits)
            .encode(sig_algo)
            .encode(sig, BIT_STRING)
         .end_cons()
      .get_contents()
      );

   return PKCS10_Request(source);
   }

}

}
