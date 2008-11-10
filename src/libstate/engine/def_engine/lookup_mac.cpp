/*************************************************
* MAC Lookup                                     *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/def_eng.h>
#include <botan/scan_name.h>
#include <botan/algo_factory.h>
#include <botan/lookup.h>

#if defined(BOTAN_HAS_CBC_MAC)
  #include <botan/cbc_mac.h>
#endif

#if defined(BOTAN_HAS_CMAC)
  #include <botan/cmac.h>
#endif

#if defined(BOTAN_HAS_HMAC)
  #include <botan/hmac.h>
#endif

#if defined(BOTAN_HAS_SSL3_MAC)
  #include <botan/ssl3_mac.h>
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
  #include <botan/x919_mac.h>
#endif

namespace Botan {

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
MessageAuthenticationCode*
Default_Engine::find_mac(const SCAN_Name& request,
                         Algorithm_Factory& af) const
   {

#if defined(BOTAN_HAS_CBC_MAC)
   if(request.algo_name() == "CBC-MAC" && request.arg_count() == 1)
      return new CBC_MAC(get_block_cipher(request.argument(0)));
#endif

#if defined(BOTAN_HAS_CMAC)
   if(request.algo_name() == "CMAC" && request.arg_count() == 1)
      return new CMAC(get_block_cipher(request.argument(0)));
#endif

#if defined(BOTAN_HAS_HMAC)
   if(request.algo_name() == "HMAC" && request.arg_count() == 1)
      return new HMAC(af.make_hash_function(request.argument(0)));
#endif

#if defined(BOTAN_HAS_SSL3_MAC)
   if(request.algo_name() == "SSL3-MAC" && request.arg_count() == 1)
      return new SSL3_MAC(af.make_hash_function(request.argument(0)));
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
   if(request.algo_name() == "X9.19-MAC" && request.arg_count() == 0)
      return new ANSI_X919_MAC(get_block_cipher("DES"));
#endif

   return 0;
   }

}
