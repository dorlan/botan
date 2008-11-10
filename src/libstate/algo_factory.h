/**
* Algorithm Factory
* (C) 2008 Jack Lloyd
*/

#ifndef BOTAN_ALGORITHM_FACTORY_H__
#define BOTAN_ALGORITHM_FACTORY_H__

#include <botan/scan_name.h>
#include <botan/mutex.h>
#include <string>
#include <vector>

namespace Botan {

class HashFunction;
class MessageAuthenticationCode;

/**
* Algorithm Factory
*/
class BOTAN_DLL Algorithm_Factory
   {
   public:
      ~Algorithm_Factory();

      void add_engine(class Engine*);

      class BOTAN_DLL Engine_Iterator
         {
         public:
            class Engine* next() { return af.get_engine_n(n++); }
            Engine_Iterator(const Algorithm_Factory& a) : af(a) { n = 0; }
         private:
            const Algorithm_Factory& af;
            u32bit n;
         };
      friend class Engine_Iterator;

      // Hash function operations
      const HashFunction* prototype_hash_function(const SCAN_Name& request);
      HashFunction* make_hash_function(const SCAN_Name& request);
      void add_hash_function(HashFunction* hash);

      // MAC operations
      const MessageAuthenticationCode* prototype_mac(const SCAN_Name& request);
      MessageAuthenticationCode* make_mac(const SCAN_Name& request);
      void add_mac(MessageAuthenticationCode* mac);

   private:
      class Engine* get_engine_n(u32bit) const;

      std::vector<class Engine*> engines;
   };

}

#endif
