#include <tree/tree.hpp>
#include <xoshiro256starstar/xoshiro256starstar.hpp>

#include <algorithm>
#include <iterator>

template <typename T> struct cmp {
  T a;
  auto operator()(T x) { return x <=> a; };
};

template <typename T> cmp<T> make_cmp(T a) { return cmp<T>{a}; }

// Iterate from beginning to end
template <typename T> bool verify_size(tree<T> &dictionary) {
  std::size_t count{};
  for (auto iter = dictionary.begin(); iter != dictionary.end(); ++iter) {
    ++count;
    if (count > dictionary.size()) { break; }
  }
  if (count != dictionary.size()) {
    std::printf("Tree counting test failed, reached %d/%d\n", (int)count,
      (int)dictionary.size());
    return false;
  }
  // Check that prev(begin) == end
  if (auto prev = std::prev(dictionary.begin()); prev != dictionary.end()) {
    std::printf("Tree iterator circularity test failed\n");
    return false;
  }
  return true;
}

bool test_small_trees() {
  // For each small tree, can iterate from beginning to end,
  // exchange any pair of items, erase any single item, then iterate again
  for (std::size_t size = 1; size != 7; ++size) {
    for (std::size_t i = 0; i != size; ++i) {
      for (std::size_t j = 0; j != size; ++j) {
        if (size == 1 || i != j) {
          for (int pattern = 0; pattern != 1 << size; ++pattern) {
            for (std::size_t k = 0; k != size; ++k) {
              tree<int> dictionary;
              // Insert items according to pattern
              for (std::size_t l = 0; l != size; ++l) {
                if ((pattern >> l) & 1) {
                  dictionary.insert(dictionary.end(), l);
                } else {
                  dictionary.insert(dictionary.begin(), l);
                }
              }
              // Check iteration
              if (!verify_size(dictionary)) {
                std::printf(
                  "Small tree with size %d and pattern %d, cannot iterate\n",
                  (int)size, (int)pattern);
                return false;
              }
              if (size > 1) {
                // Exchange a pair of items
                auto iter = std::next(dictionary.begin(), i);
                auto jter = std::next(dictionary.begin(), j);
                dictionary.exchange_elements(iter, jter);
              }
              // Erase an item
              auto kter = std::next(dictionary.begin(), k);
              dictionary.erase(kter);
              // Check iteration
              if (!verify_size(dictionary)) {
                std::printf(
                  "Small tree with size %d and pattern %d, cannot iterate\n"
                  "  after exchanging %d, %d and erasing %d\n",
                  (int)size, (int)pattern, (int)i, (int)j, (int)k);
                return false;
              }
            }
          }
        }
      }
    }
  }
  return true;
}

bool items_are_in_ascending_order(auto &container) {
  float x0 = -std::numeric_limits<float>::infinity();
  for (float x: container) {
    if (x < x0) { return false; }
    x0 = x;
  }
  return true;
}

bool test_equal_range(auto &urbg, bool do_shuffle) {
  std::printf("Test equal range with two comparators\n");
  tree<int> dictionary;
  // Insert the numbers 1 to 100 into the tree, in random order
  std::vector<int> values(100);
  std::iota(values.begin(), values.end(), 1);
  if (do_shuffle) { std::ranges::shuffle(values, urbg); }
  for (auto value: values) {
    dictionary.insert(dictionary.lower_bound(make_cmp(value)), value);
  }
  // Find the numbers from 40 to 60 (inclusive) in the tree
  auto [begin, end] = dictionary.equal_range(make_cmp(40), make_cmp(60));
  // Verify that the range contains 21 numbers from 40 to 60 (inclusive)
  bool ok = true;
  std::size_t count{};
  for (auto value: std::ranges::subrange(begin, end)) {
    if (value < 40) {
      ok = false;
      std::printf("  found %d (too small)\n", value);
    } else if (value > 60) {
      ok = false;
      std::printf("  found %d (too great)\n", value);
    }
    ++count;
  }
  if (count != 21) {
    ok = false;
    std::printf("  found %d numbers (should be 21)\n", (int)count);
  }

  return ok;
}

int main() {
  bool ok = true;
  std::printf("Small dictionary tests\n");

  if (test_small_trees()) {
    std::printf("  build, iterate, exchange, erase for small trees - ok\n");
  } else {
    std::printf("  build, iterate, exchange, erase for small trees - fail\n");
    ok = false;
  }

  std::printf("Large dictionary tests\n");

  constexpr std::size_t repeat_count{64};
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
    // Insert randomly selected values
    for (int j = 0; j != point_count; ++j) {
      auto a = dist(urbg);
      dictionary.insert(dictionary.lower_bound(make_cmp(a)), a);
      ++insertions;

      if (dictionary.size() != insertions - deletions) { goto out; }
    }

    // Erase items between two randomly selected values
    auto a = dist(urbg);
    auto b = dist(urbg);
    if (a > b) { std::swap(a, b); }
    auto [iter, jter] = dictionary.equal_range(make_cmp(a), make_cmp(b));
    while (iter != jter) {
      iter = dictionary.erase(iter);
      ++deletions;
      if (dictionary.size() != insertions - deletions) { goto out; }
    }
  }

out:

  if (items_are_in_ascending_order(dictionary)) {
    std::printf("  item order test ok\n");
  } else {
    std::printf("  item order test failed\n");
    ok = false;
  }

  std::printf("  insertions %llu, deletions %llu, size %llu\n", insertions,
    deletions, dictionary.size());

  if (dictionary.size() == insertions - deletions) {
    std::printf("  size ok\n");
  } else {
    std::printf("  size does not match, should be %llu\n", insertions - deletions);
    ok = false;
  }

  if (!dictionary.empty()) {
    std::vector<float> elements;
    elements.reserve(dictionary.size());

    std::ranges::copy(dictionary, std::back_inserter(elements));

    auto tree_iter = dictionary.end();
    auto vector_iter = elements.end();
    std::size_t step{};
    while (tree_iter != dictionary.begin()) {
      --tree_iter;
      --vector_iter;
      if (*tree_iter != *vector_iter) { break; }
      ++step;
    }
    if (step != dictionary.size()) {
      std::printf("  iterator decrement test failed at step %d\n", (int)step);
      ok = false;
    } else if (tree_iter != dictionary.begin()) {
      std::printf(
        "  iterator decrement test failed, did not reach beginning\n");
      ok = false;
    } else {
      std::printf("  iterator decrement test succeeded\n");
    }
  }

  ok = ok && test_equal_range(urbg, true);
  ok = ok && test_equal_range(urbg, false);

  return ok ? 0 : 1;
}
