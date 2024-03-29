#include "memsim.h"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

namespace {
struct Timer {
  // return elapsed time (in seconds) since last reset/or construction
  // reset_p = true will reset the time
  double elapsed(bool reset_p = false);
  // reset the time to 0
  void reset();
  Timer();
  ~Timer();

  private:
  struct Pimpl;
  Pimpl * pimpl_;
};

struct Timer::Pimpl {
  std::chrono::time_point<std::chrono::steady_clock> start;
};
Timer::Timer()
{
  pimpl_ = new Pimpl;
  reset();
}
Timer::~Timer() { delete pimpl_; }
double Timer::elapsed(bool reset_p)
{
  double result = 1e-6
      * std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - pimpl_->start)
            .count();
  if (reset_p) reset();
  return result;
}
void Timer::reset() { pimpl_->start = std::chrono::steady_clock::now(); }

typedef std::vector<std::string> vs_t;

// split string p_line into a vector of strings (words)
// the delimiters are 1 or more whitespaces
vs_t split(const std::string & p_line)
{
  auto line = p_line + " ";
  vs_t res;
  bool in_str = false;
  std::string curr_word = "";
  for (auto c : line) {
    if (isspace(c)) {
      if (in_str) res.push_back(curr_word);
      in_str = false;
      curr_word = "";
    } else {
      curr_word.push_back(c);
      in_str = true;
    }
  }
  return res;
}

// convert string to long
// if successful, success = True, otherwise success = False
long str2long(const std::string & s, bool & success)
{
  char * end = 0;
  errno = 0;
  long res = strtol(s.c_str(), &end, 10);
  if (*end != 0 || errno != 0) {
    success = false;
    return -1;
  }
  success = true;
  return res;
}

std::string stdin_readline()
{
  std::string result;
  while (1) {
    int c = fgetc(stdin);
    if (c == -1) break;
    result.push_back(c);
    if (c == '\n') break;
  }
  return result;
}

std::string join(const vs_t & toks, const std::string & sep = " ")
{
  std::string res;
  bool first = true;
  for (auto & t : toks) {
    res += (first ? "" : sep) + t;
    first = false;
  }
  return res;
}

void parse_request(long line_no, vs_t & toks, Request & request)
{
  auto line_err = [&] {
    printf("Error on line %ld: \"%s\"\n", line_no, join(toks).c_str());
    exit(-1);
  };

  if (toks.size() > 2) line_err();

  // convert first word into number
  bool ok;
  long tag = str2long(toks[0], ok);
  if (! ok) line_err();

  if (tag < 0) {
    if (tag < -10000000 || toks.size() != 1) line_err();
    request = { int(tag), 0 };
    return;
  }
  if (tag > 10000000 || toks.size() != 2) line_err();
  long size = str2long(toks[1].c_str(), ok);
  if (! ok || size < 1 || size > 10000000) line_err();
  request = { int(tag), int(size) };
}

void usage(const std::string & pname)
{
  printf("Usage: %s <page-size>\n", pname.c_str());
  printf("   where page-size is int in range [1..1,000,000]\n");
  exit(-1);
}
} // anonymous namespace

int main(int argc, char ** argv)
{
  // parse command line arguments
  // ------------------------------
  if (argc != 2) usage(argv[0]);
  bool ok;
  long page_size = str2long(argv[1], ok);
  if (! ok || page_size < 1 || page_size > 1000000) {
    printf("Bad page size '%s'.\n", argv[1]);
    usage(argv[0]);
  }

  std::vector<Request> requests;
  long line_no = 0;
  while (true) {
    line_no++;
    // get next line
    auto line = stdin_readline();
    if (line.size() == 0) break;
    // tokenize line
    auto toks = split(line);
    // skip empty lines
    if (toks.size() == 0) continue;
    // convert toks into request
    Request request;
    parse_request(line_no, toks, request);
    requests.push_back(request);
  }

  // call simulator
  Timer t;
  MemSimResult results = mem_sim(page_size, requests);
  auto elapsed = t.elapsed();

  // report results
  printf("\n----- Results ---------------------------------\n");
  printf("pages requested:                %ld\n", long(results.n_pages_requested));
  printf("largest free partition size:    %ld\n", long(results.max_free_partition_size));
  printf("largest free partition address: %ld\n", long(results.max_free_partition_address));
  printf("elapsed time:                   %.3lfs\n", elapsed);
  return 0;
}
