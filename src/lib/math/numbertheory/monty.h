/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MONTY_INT_H_
#define BOTAN_MONTY_INT_H_

#include <botan/bigint.h>

namespace Botan {

class Modular_Reducer;

class Montgomery_Params;

/**
* The Montgomery representation of an integer
*/
class Montgomery_Int final
   {
   public:
      /**
      * Create a zero-initialized Montgomery_Int
      */
      Montgomery_Int(std::shared_ptr<const Montgomery_Params> params) : m_params(params) {}

      /**
      * Create a Montgomery_Int
      */
      Montgomery_Int(std::shared_ptr<const Montgomery_Params> params,
                     const BigInt& v,
                     bool redc_needed = true);

      bool operator==(const Montgomery_Int& other) const;
      bool operator!=(const Montgomery_Int& other) const { return (m_v != other.m_v); }

      std::vector<uint8_t> serialize() const;

      size_t size() const;
      bool is_one() const;
      bool is_zero() const;

      void fix_size();

      /**
      * Return the value to normal mod-p space
      */
      BigInt value() const;

      /**
      * Return the Montgomery representation
      */
      const BigInt& repr() const { return m_v; }

      Montgomery_Int operator+(const Montgomery_Int& other) const;

      Montgomery_Int operator-(const Montgomery_Int& other) const;

      Montgomery_Int& operator+=(const Montgomery_Int& other);

      Montgomery_Int& operator-=(const Montgomery_Int& other);

      Montgomery_Int operator*(const Montgomery_Int& other) const;

      Montgomery_Int& operator*=(const Montgomery_Int& other);

      Montgomery_Int& operator*=(const secure_vector<word>& other);

      Montgomery_Int square() const;

      Montgomery_Int& square_this();

      Montgomery_Int multiplicative_inverse() const;

      Montgomery_Int additive_inverse() const;

      Montgomery_Int& mul_by_2(secure_vector<word>& ws);

      Montgomery_Int& mul_by_3(secure_vector<word>& ws);

      Montgomery_Int& mul_by_4(secure_vector<word>& ws);

      Montgomery_Int& mul_by_8(secure_vector<word>& ws);

   private:
      std::shared_ptr<const Montgomery_Params> m_params;
      BigInt m_v;
   };

/**
* Parameters for Montgomery Reduction
*/
class Montgomery_Params final
   {
   public:
      /**
      * Initialize a set of Montgomery reduction parameters. These values
      * can be shared by all values in a specific Montgomery domain.
      */
      Montgomery_Params(const BigInt& p, const Modular_Reducer& mod_p);

      const BigInt& p() const { return m_p; }
      const BigInt& R1() const { return m_r1; }
      const BigInt& R2() const { return m_r2; }
      const BigInt& R3() const { return m_r3; }

      word p_dash() const { return m_p_dash; }

      size_t p_words() const { return m_p_words; }

      BigInt redc(const BigInt& x) const;

      BigInt mul(const BigInt& x, const BigInt& y) const;

      BigInt mul(const BigInt& x, const secure_vector<word>& y) const;

      BigInt sqr(const BigInt& x) const;

      BigInt inv_mod_p(const BigInt& x) const;

   private:
      BigInt m_p;
      BigInt m_r1;
      BigInt m_r2;
      BigInt m_r3;
      word m_p_dash;
      size_t m_p_words;
   };

}

#endif
