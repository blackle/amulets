#include "llama.h"

#include <vector>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstring>
#include <fstream>
#include <string>
#include <regex>
#include <thread>

#include "concurrentqueue.h"
#include "SHA256.h"

#define SHA256_BYTES 32U

#include <signal.h>
#include <math.h>

namespace chrono = std::chrono;

chrono::time_point<chrono::steady_clock> my_start;

// Number of SHA256 hash workers
#define MAX_THREADS 12

int num_workers = 0;
int worker_id = 0;

static std::regex no_use_words("\\b(two|three|four|five|six|seven|ten|eleven|eighty|ninety|seventy|sixty|hundred|in sha)\\b|teen\\b|0x|[134789]|hashtag|m a hash|is a hash|eight-fold|eightfold|m a sha|eight in twelve|eight.bit|eleventh|eighth|twelfth", std::regex_constants::icase);

static std::regex requirements("twelve eights", std::regex_constants::icase);

struct ministring {
    size_t size;
    char data[63];
};

moodycamel::ConcurrentQueue<ministring> my_queue;

int sha256_count_eights_impl(unsigned char* out)
{
    // Use fast memmem to rule out small amulets quickly
    if (memmem(out, SHA256_BYTES, "\x88\x88\x88", 3) == NULL) {
        return 0;
    }
    int max = 0;
    int curr = 0;
    for (int i = 0; i < SHA256_BYTES; i++) {
        unsigned char byte = out[i];
        bool cond1 = (byte & 0xF0) == 0x80;
        bool cond2 = (byte & 0x0F) == 0x08;
        if (cond1) {
            curr += 1;
            if (curr > max) {
                max = curr;
            }
        } else {
            curr = 0;
        }
        if (cond2) {
            curr += 1;
            if (curr > max) {
                max = curr;
            }
        } else {
            curr = 0;
        }
    }
    return max;
}

int sha256_count_eights(const char* input, size_t size)
{
    SHA256 sha;
    sha.update((const uint8_t*)input, size);
    sha.pad();
    unsigned char out[SHA256_BYTES];
    sha.revert(out);

    return sha256_count_eights_impl(out);
}

const char* prompt = 
"\"In the hash of me, there are twelve eights\"\n"
"\"This is an amulet, a poem you can hash, with twelve eights\"\n"
"\"Hash this amulet, with sha256, for twelve eights\"\n"
"\"Get out your sha256 tool, and count all twelve eights\"\n"
"\"Behold, all twelve eights, in this amulet's hash\"\n"
"\"See the twelve eights, in my hash, glitter\"\n"
"\"Hash me with sha-256, if you dare, glimpse twelve eights\"\n"
"\"Take a look, at twelve eights in my sha-256, please\"\n"
"\"Please, sha256 hash me, and see twelve eights\"\n"
"\"";

static std::vector<std::string> starters = {
    "I'm", "Shining", "Amulet", "See", "This", "Hash", "Crypto", "What", "How", "When", "Where", "Why", "Who", "Wow", "Math", "Ruby", "SHA256", "SHA-256", "Power", "Help", "Fate", "Fortune", "Calculate", "Calculator", "Digit", "Luck", "Joy", "Emerald", "Gold", "Beauty", "Beautiful", "Behold", "Awe", "Chance", "Flit", "Flutter", "Difficult", "Rare", "Hello", "Found" };

static long long total_candidates = 0;

void quitHandler(int dummy) {
    auto end = chrono::steady_clock::now(); 
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - my_start).count();
    float mhs = total_candidates / duration / 1000;
    std::cout << "total_candidates = " << total_candidates << " duration = " << duration << "ms" << " mhs = " << mhs << " queue_size = " << my_queue.size_approx() << std::endl;
}

void intHandler(int dummy) {
    quitHandler(dummy);
    abort();
}

std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string joinString(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::string result;

    for (auto it = strings.begin(); it != strings.end(); ++it) {
        result += *it;

        if (it != strings.end() - 1) {
            result += delimiter;
        }
    }

    return result;
}

#define BATCH_SIZE 16384
void* thread_function(void* data) {
    std::vector<ministring> batch;
    batch.reserve(BATCH_SIZE);
    while (true) {
        size_t data_count = my_queue.try_dequeue_bulk(batch.data(), BATCH_SIZE);
        for (int i = 0; i < data_count; i++) {
            int count = sha256_count_eights(batch[i].data, batch[i].size);
            if (count > 9) {
                std::string str(batch[i].data, batch[i].size);
                auto splits = splitString(str, "\n");
                auto out = joinString(splits, "\\n");
                std::cout << count << " " << out << std::endl;
            }
        }
        if (data_count == 0) {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }
    return NULL;
}

void check_candidate(const ministring& mystring) {
    // std::cout << str << std::endl << std::endl;
    if (mystring.size > 64) {
        return;
    }
    total_candidates++;
    // ministring mystring;
    // mystring.size = str.size();
    // memcpy(mystring.data, str.c_str(), str.size());
    if (my_queue.enqueue(mystring) == false) {
        perror("couldn't push :(");
        abort();
    }
}

static const std::vector<std::string> hearts = {"", "<3", " <3", "♥", " ♥", "♥️", " ♥️", " :3", " :D", " >:3", " >;3c", " >:3c", " ;3", " >;3", " ^^", " ^w^", " owo", " OwO", " uwu", " UwU", " >w<", " \\o/"};
void process_candidate2_1(ministring& mystring) {
    for (int j = 0; j < hearts.size(); j++) {
        if (mystring.size + hearts[j].size() > 64) {
            continue;
        }
        size_t old_size = mystring.size;
        size_t ending_size = hearts[j].size();
        memcpy(mystring.data + mystring.size, hearts[j].c_str(), ending_size);
        mystring.size += ending_size;
        check_candidate(mystring);
        mystring.size = old_size;
    }
}

static const std::vector<std::string> endings = {"", ".", "!", "...", "..", "...!", "..!", "!?", "?!", "!!", "!!!", "~!", "!~", "!!~", "!!!~"};
void process_candidate2(const std::string& str) {
    ministring mystring;
    mystring.size = str.size();
    memcpy(mystring.data, str.c_str(), str.size());

    for (int j = 0; j < endings.size(); j++) {
        if (mystring.size + endings[j].size() > 64) {
            continue;
        }
        size_t old_size = mystring.size;
        size_t ending_size = endings[j].size();
        memcpy(mystring.data + mystring.size, endings[j].c_str(), ending_size);
        mystring.size += ending_size;
        process_candidate2_1(mystring);
        mystring.size = old_size;
    }
}


static const std::vector<std::string> slash_replacements = {", / ", " / ", ". / ", ".. / ", "... / ", "! / "};
static const std::vector<std::string> comma_replacements = {": ", "; ", ";\n", ",\n", "\n", ":\n", ", ", "—"};
static const std::vector<std::string> comma_replacements_no_colon = {",\n", "\n", ", ", "—"};
void recurse_replacements(const std::string& previous, const std::vector<std::string>& splits, int split, const std::vector<std::string>& replacements) {

    if (split == splits.size() - 1) {
        std::string next = previous + splits[split];
        process_candidate2(next);
        return;
    }

    for (int i = 0; i < replacements.size(); i++) {
        std::string next = previous + splits[split] + replacements[i];
        if (replacements[i][0] == ':' || replacements[i][0] == ';') {
            recurse_replacements(next, splits, split + 1, comma_replacements_no_colon);
        } else {
            recurse_replacements(next, splits, split + 1, replacements);
        }
    }
}

void process_candidate3(const std::string& str) {
    while (my_queue.size_approx() > 5000000) {
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }
    auto splits = splitString(str, ", ");
    recurse_replacements("", splits, 0, comma_replacements);
    recurse_replacements("", splits, 0, slash_replacements);
}

void case_recurse(char* str, int idx, int length, bool wasspace, bool force) {
    if (idx == length) {
        process_candidate3(std::string(str));
    } else {
        int alpha = isalpha(str[idx]);
        if (alpha == 0 || wasspace == false) {
            char old = str[idx];
            if (alpha != 0 && force) {
                str[idx] = toupper(str[idx]);
            }
            wasspace = str[idx] == ' ';
            case_recurse(str, idx+1, length, wasspace, wasspace ? false : force);
            str[idx] = old;
        } else {
            str[idx] = toupper(str[idx]);
            if (idx+1 != length && isalpha(str[idx+1]) != 0) {
                case_recurse(str, idx+1, length, false, true);
            }
            case_recurse(str, idx+1, length, false, false);
            str[idx] = tolower(str[idx]);
            case_recurse(str, idx+1, length, false, false);
        }
    }
}


void process_candidate(const std::string& str) {
    if (!std::regex_search(str, requirements)) {
        return;
    }
    char* cstr = strdup(str.c_str());
    case_recurse(cstr, 0, strlen(cstr), true, false);
    free(cstr);
}

int num_occurrences(const std::string& str, char c) {
  int count = 0;
  for (char ch : str) {
    if (ch == c) {
      count++;
    }
  }
  return count;
}

void recurse_prompt(llama_context* ctx, int breadth, int max_depth, int n_past, const std::string& previous) {
    if (std::regex_search(previous, no_use_words)) {
        return;
    }

    if (num_occurrences(previous, '-') > 2) {
        return;
    }

    if (max_depth == n_past || previous.length() > 64) {
        return;
    }
    auto logits = llama_get_logits(ctx);
    auto n_vocab = llama_n_vocab(ctx);
    std::vector<llama_token_data> candidates;
    candidates.reserve(n_vocab);
    for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
        candidates.emplace_back(llama_token_data{token_id, logits[token_id], 0.0f});
    }
    std::sort(candidates.begin(), candidates.end(), [](const llama_token_data & a, const llama_token_data & b) {
        return a.logit > b.logit;
    });
    float max_l = candidates[0].logit;
    float sum = 0;
    for (int i = 0; i < candidates.size(); i++) {
        float p = expf(candidates[i].logit - max_l);
        sum += p;
        candidates[i].p = p;
    }
    for (int i = 0; i < candidates.size(); i++) {
        candidates[i].p /= sum;
    }
    // std::sort(candidates.begin(), candidates.end(), [](const llama_token_data & a, const llama_token_data & b) {
    //     return a.p > b.p;
    // });

    auto n_threads = 4;
    bool saw_ending = false;

    float cum = 0;
    int mybreadth = (previous.length() > 40) ? 1 : breadth;
    bool all = (previous.length() == 0);
    for (int i = 0; ((i < mybreadth) && (cum < .8)) || (all && i < candidates.size()); cum += candidates[i++].p) {
        if (all && (i % num_workers) != worker_id) {
            continue;
        }
        auto next_token = candidates[i].id;
        if (next_token == llama_token_eos() || next_token == llama_token_nl() || next_token == llama_token_bos()) {
            continue;
        }
        std::string tok = llama_token_to_str(ctx, next_token);
        if (tok.find("\"") != std::string::npos) {
            if (saw_ending == false) {
                saw_ending = true;
                // std::cout << previous << std::endl;
                process_candidate(previous);
            }
            continue;
        }
        if (tok.find("#") != std::string::npos || tok.find(":") != std::string::npos) {
            continue;
        }
        if (previous.length() == 0 && tok[0] == ' ') {
            continue;
        }
        if (llama_eval(ctx, &next_token, 1, n_past, n_threads)) {
            fprintf(stderr, "\n%s : failed to evaluate\n", __func__);
            return;
        }
        std::string next = previous + tok;
        recurse_prompt(ctx, breadth, max_depth, n_past + 1, next);
    }
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s num_workers worker_id", argv[0]);
        return -1;
    }

    num_workers = atoi(argv[1]);
    worker_id = atoi(argv[2]);

    pthread_t threads[MAX_THREADS];
    for (int thread_id = 0; thread_id < MAX_THREADS; thread_id++) {
        pthread_create(&threads[thread_id], NULL, &thread_function, NULL);
    }

    auto lparams = llama_context_default_params();
    signal(SIGINT, intHandler);
    signal(SIGQUIT, quitHandler);

    printf("%s\n", prompt);

    lparams.n_ctx     = 1024;
    lparams.seed      = 0;
    lparams.f16_kv    = true;
    lparams.use_mmap  = true;
    lparams.use_mlock = true;

    auto n_threads = 4;
    auto n_past = 0;
    auto n_predict = 1;

    auto model = llama_load_model_from_file("llama-2-7b.ggmlv3.q4_K_M.bin", lparams);
    auto ctx = llama_new_context_with_model(model, lparams);
    auto tokens = std::vector<llama_token>(lparams.n_ctx);
    auto n_prompt_tokens = llama_tokenize(ctx, prompt, tokens.data(), tokens.size(), true);

    if (n_prompt_tokens < 1) {
        fprintf(stderr, "%s : failed to tokenize prompt\n", __func__);
        return 1;
    }

    // evaluate prompt
    llama_eval(ctx, tokens.data(), n_prompt_tokens, n_past, n_threads);
    n_past = n_prompt_tokens;

    my_start = chrono::steady_clock::now();
    std::cout << "done parsing prompt" << std::endl;

    recurse_prompt(ctx, 8, n_past + 24, n_past, "");

    std::cout << "total_candidates = " << total_candidates << std::endl;

    return 0;
}
