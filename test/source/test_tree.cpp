#include <tree/tree.hpp>
#include <xoshiro256starstar/xoshiro256starstar.hpp>

template <typename T> struct cmp {
  T a;
  auto operator()(T x) { return x <=> a; };
};

template <typename T> cmp<T> make_cmp(T a) { return cmp<T>{a}; }

int main() {
  constexpr std::size_t repeat_count{32};
  constexpr std::size_t point_count{65536};

  tree<float> dictionary;

  // auto urbg = xoshiro256starstar::xoshiro256starstar{1ull};
  auto urbg = xoshiro256starstar::xoshiro256starstar{
    xoshiro256starstar::seed_from_urbg,
    std::random_device{},
  };

  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  std::size_t insertions{};
  std::size_t deletions{};

  for (int i = 0; i != repeat_count; ++i) {
    for (int j = 0; j != point_count; ++j) {
      auto a = dist(urbg);
      dictionary.insert(dictionary.lower_bound(make_cmp(a)), a);
      ++insertions;

      if (dictionary.size() != insertions - deletions) { goto out; }
    }
    auto a = dist(urbg);
    auto b = dist(urbg);
    if (a > b) { std::swap(a, b); }
    auto iter = dictionary.lower_bound(make_cmp(a));
    auto jter = dictionary.upper_bound(make_cmp(b));
    while (iter != jter) {
      iter = dictionary.erase(iter);
      ++deletions;
      if (dictionary.size() != insertions - deletions) { goto out; }
    }
  }

out:

  bool ok = true;
  float prev = -std::numeric_limits<float>::infinity();
  for (auto x: dictionary) {
    if (x < prev) {
      ok = false;
      break;
    }
  }

  std::printf("Dictionary test %s\n"
              "  insertions %llu, deletions %llu, size %llu\n",
    (ok ? "succeeded" : "failed"), insertions, deletions, dictionary.size());

  if (dictionary.size() == insertions - deletions) {
    std::printf("  size ok\n");
  } else {
    std::printf("  size does not match, should be %llu\n", insertions - deletions);
  }

  return ok ? 0 : 1;
}
