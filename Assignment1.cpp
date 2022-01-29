// COP 4520
// Spring 2022
// Author: William Quiroga

#include <bits/stdc++.h>

using namespace std;

using ll = long long;

const int N = (int) 1e8;
const int sqrtN = (int) 1e4; // 10^4 * 10^4 = 10^8
const int NUM_THREADS = 8;
const int PRIMES_DESIRED = 10;

bool isComp[N + 1];
// vector<bool> isComp(N + 1);
vector<int> primes;

// mark all multiples of each prime in the range [l, r] inclusive as composite
// This makes the work for each thread almost exactly the same, since the position
// of the range has nothing to do with the runtime of this function, only the size
// of the range matters here. This is because the for loop only loops from around
// the start of the range to the end of the range in jumps of the length of each prime.
// And since we made each range the exact same length, we know that each thread is doing
// approximately the same amount of work.
void checkPrimes(int l, int r) {

  for(int i = 0; i < primes.size(); i++) {
    int prime = primes[i];

    // get the first multiple of the prime >= l
    int start = l - (l % prime);
    if(start < l) start += prime;

    // mark all multiples of this prime as composite
    for(int i = start; i <= r; i += prime) {
      isComp[i] = true;
    }
  }

}

int main() {

  // First find the primes up to sqrt(N)
  // We only need the primes up to sqrt(N) because every number up to N
  // must have a factor <= sqrt(N) if it isn't prime, which means it must
  // have a prime factor that is <= sqrt(N).

  // Proof:
  // Suppose x is some composite number on the range 1 to N. That means
  // there exists two integers greater than 1, let's say a and b, such that
  // x = a * b.
  // Assume that a > sqrt(N) and b > sqrt(N). That means we can represent them as
  // a = sqrt(N) + A and b = sqrt(N) + B, where A and B are positive integers.
  // This means that a * b = (sqrt(N) + A) * (sqrt(N) + B) =
  // sqrt(N)^2 + A*sqrt(N) + B*sqrt(N) + A*B = 
  // N + A*sqrt(N) + B*sqrt(N) + A*B.
  // Since A, B, and the sqrt(N) are positive integers, we know that
  // A*sqrt(N) + B*sqrt(N) + A*B > 0. Let C represent that value.
  // So we know that a * b = N + C. This is a problem, since we know that
  // a * b = x <= N < N + C. Thus we have reached a contradiction.
  // Therefore, x must have a factor <= sqrt(N), meaning x has a prime factor <= sqrt(N).
  

  // set 0 and 1 to be composite
  // (although 0 and 1 aren't composite, it's easier to just assume they are here)
  isComp[0] = isComp[1] = true;

  // run through all possible candidates up to sqrt(N)
  for(int i = 2; i <= sqrtN; i++) {
    if(isComp[i]) continue;
    // i is prime here
    primes.push_back(i);

    // mark all multiples of i as composite
    for(int j = i + i; j <= sqrtN; j += i)
      isComp[j] = true;
  }


  // give each thread a dedicated range of values to take care of
  vector<pair<int, int>> rangePerThread;

  // 8 divides (N - sqrtN) in this specific case, so it works out nicely
  int lengthOfRange = (N - sqrtN) / NUM_THREADS;

  // start at sqrt(N) + 1 since we have taken care of everything <= sqrt(N)
  int start = sqrtN + 1;
  for(int i = 0; i < NUM_THREADS; i++) {
    rangePerThread.emplace_back(start, start + lengthOfRange - 1);
    start += lengthOfRange;
  }

  // Get the current time before thread spawning
  auto timeStart = chrono::system_clock::now();

  vector<thread> threads;
  for(int i = 0; i < NUM_THREADS; i++) {
    auto [l, r] = rangePerThread[i];
    // give each thread it's dedicated range to check
    threads.push_back(thread(checkPrimes, l, r));
  }

  // join all the threads.
  for(int i = 0; i < NUM_THREADS; i++) {
    threads[i].join();
  }

  // Get the current time after the threads have finished
  auto timeEnd = chrono::system_clock::now();

  // The duration should be the ending time - starting time.
  chrono::duration<double> elapsed = timeEnd - timeStart;

  int primeCount = 0;
  long long primeSum = 0;
  vector<int> largestPrimes;

  for(int i = 0; i <= N; i++) {
    if(!isComp[i]) {
      // if this current number is prime, add to the count and the sum
      primeCount++;
      primeSum += i;
    }
  }

  // get the 10 highest primes
  for(int i = N; largestPrimes.size() < PRIMES_DESIRED; i--) {
    if(isComp[i]) continue;
    largestPrimes.push_back(i);
  }

  // reverse the order since I got them highest to lowest
  reverse(largestPrimes.begin(), largestPrimes.end());

  freopen("primes.txt", "w", stdout);

  // print elapsed time, number of primes, sum of primes, and the 10 largest primes.
  cout << elapsed.count() << ' ' << primeCount << ' ' << primeSum << '\n';
  for(int prime : largestPrimes) {
    cout << prime << ' ';
  }
  cout << endl;

  // I'm using the Sieve of Eratosthenes to compute the answer.
  // The time complexity of the Sieve of Eratosthenes is O(n log log n)
  // I first find the first sqrt(N) prime using the sieve method. This gives us
  // O(sqrt(N) log log sqrt(N)) = O(sqrt(N) log 1/2 log N) = O(sqrt(N) log log N).
  // Then we divide the work amongst the 8 threads (almost) equally by giving each thread
  // a range to run the sieve method on, so each thread is doing O((N log log N) / 8) =
  // O(N log log N) work. Then to find the number of primes and the sum of the primes, I do
  // an O(N) loop to get those values, so overall the time complexity is
  // O(sqrt(N) log log N + N log log N + N) = O((sqrt(N) + N) log log N).

  // I experimented with just running a naive Sieve of Eratosthenes on the whole range (1, N)
  // and found that I'm saving 0.3 to 0.4 seconds on my computer.

  return 0;
}
