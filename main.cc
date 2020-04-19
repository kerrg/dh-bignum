#include <fstream>
#include <iostream>

#include "ttmath/ttmath/ttmath.h"

namespace {

constexpr size_t kBigIntSize = 512;

uint64_t AskKernelForRandom() {
  std::ifstream urand_file("/dev/urandom", std::ios::in | std::ios::binary);

  uint64_t result;
  urand_file.read(reinterpret_cast<char*>(&result), sizeof(result));

  return result;
}

// Calculate a ^ m mod n
ttmath::Int<kBigIntSize> compute(ttmath::Int<kBigIntSize> x,
                                 ttmath::Int<2> y,
                                 ttmath::Int<kBigIntSize> p) {
  ttmath::Int<kBigIntSize> result = 1;

  // In case x is more than or equal to P, should not happen given that the
  // params are fixed.
  x = x % p;

  // Again, hould not happen given the nature of DH fixed params.
  if (x == 0)
    return 0;

  while (y > 0) {
    if ((y % 2) == 1) {
      result = (result * x) % p;
    }

    y = y / 2;
    x = (x * x) % p;
  }

  return result;
}

}  // namespace

int main() {
  ttmath::Int<kBigIntSize> g, p, c;

  // These params are from https://tools.ietf.org/html/rfc5114
  p = "171254583176141379301960419792575778264088323240375085733932929816426671"
      "397476217788024387752387285929683446135893799323484756135034769321631669"
      "738132186983438164632891441853629126025225404949830905314972329658295365"
      "245072698488256583114202993359222957097432675083225259667739503949192575"
      "768420387716327420441424710535098501236058838158571626669177751934961573"
      "726561955583057270098912760065140004093658772181713883199238963093777917"
      "625906143118496429613802248519404604217104493689272529748703958739363879"
      "096722748832953774810081504758785902705917983505634881680809238046118223"
      "87520198054002990623911454389104774092183";

  g = "804136732704618930269398466502670637484460828987437442572879766950943588"
      "145914066265021583283347132847033406462850869223199940184033204619256928"
      "735199168996327965689256248477327858420804098763156962852046406953236127"
      "404737444434499665183297937831884994374166211039599577842927081922243161"
      "092735600591383693246209977007623955404285528713802680696047027732622948"
      "281800396200445376440099579097404266367569212075872614586906123644389350"
      "913614794241444555184816239146854144435570778569782574185684916123388730"
      "701742837182360812569989290496084122159334449908899602188397218524185477"
      "7608212592397013510086894908468466292313";

  ttmath::Int<2> alice_priv = AskKernelForRandom();
  ttmath::Int<kBigIntSize> alice_pub = compute(g, alice_priv, p);

  ttmath::Int<2> bob_priv = AskKernelForRandom();
  ttmath::Int<kBigIntSize> bob_pub = compute(g, bob_priv, p);

  // Pretend there's a network transfer of the public keys.
  ttmath::Int<kBigIntSize> alices_secret_key = compute(bob_pub, alice_priv, p);
  ttmath::Int<kBigIntSize> bobs_secret_key = compute(alice_pub, bob_priv, p);

  bool keys_match = alices_secret_key == bobs_secret_key;
  bool bogus_keys_do_not_match =
      compute(bob_pub, 1234, p) != compute(alice_pub, 1234, p);

  if (keys_match && bogus_keys_do_not_match) {
    std::cout << "Tests passed.\n";
    return 0;
  }

  std::cerr << "Tests failed.\n";
  return -1;
}
