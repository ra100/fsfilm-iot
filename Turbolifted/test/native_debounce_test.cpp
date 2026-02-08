#include <cassert>
#include <iostream>
#include "../src/debounce.h"

int main()
{
  Debounce db(50);
  unsigned long t = 0;
  assert(!db.sample(HIGH, t));
  t += 10;
  assert(!db.sample(LOW, t));
  t += 40;
  assert(!db.sample(HIGH, t));
  t += 100;
  assert(!db.sample(HIGH, t));
  assert(db.getState() == HIGH);

  t += 10;
  assert(!db.sample(LOW, t));
  t += 60;
  assert(db.sample(LOW, t));
  assert(db.getState() == LOW);

  std::cout << "Debounce native test passed\n";
  return 0;
}
