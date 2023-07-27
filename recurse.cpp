#include "examples/common.h"
#include "llama.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

void process_candidate(const std::string& str) {
    std::cout << str << std::endl;
}

void recurse_prompt(llama_context* ctx, int breadth, int max_depth, int n_past, const std::string& previous) {
    // Limit recursion depth and length of generated sentence
    if (max_depth == n_past || previous.length() > 64) {
        return;
    }
    // Get the model output
    auto logits = llama_get_logits(ctx);
    auto n_vocab = llama_n_vocab(ctx);
    std::vector<llama_token_data> candidates;
    candidates.reserve(n_vocab);
    for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
        candidates.emplace_back(llama_token_data{token_id, logits[token_id], 0.0f});
    }
    // Sort tokens by logit size
    std::sort(candidates.begin(), candidates.end(), [](const llama_token_data & a, const llama_token_data & b) {
        return a.logit > b.logit;
    });

    // Calculate candidate's probability value from the logit
    float max_l = candidates[0].logit;
    float sum = 0;
    for (size_t i = 0; i < candidates.size(); i++) {
        float p = expf(candidates[i].logit - max_l);
        sum += p;
        candidates[i].p = p;
    }
    for (size_t i = 0; i < candidates.size(); i++) {
        candidates[i].p /= sum;
    }

    bool saw_ending = false;
    float cumulate = 0.;

    // Iterate through the first `breadth` candidates in the top 80%
    for (int i = 0; i < breadth && cumulate < 0.8; i++) {
        cumulate += candidates[i].p;
        auto next_token = candidates[i].id;
        // Skip newlines, end of stream, and beginning of stream
        if (next_token == llama_token_eos() || next_token == llama_token_nl() || next_token == llama_token_bos()) {
            continue;
        }

        std::string tok = llama_token_to_str(ctx, next_token);
        // Check for tokens like '"', '."', '!"', etc
        if (tok.find("\"") != std::string::npos) {
            if (saw_ending == false) {
                saw_ending = true;
                process_candidate(previous);
            }
            continue;
        }

        auto n_threads = 4;
        if (llama_eval(ctx, &next_token, 1, n_past, n_threads)) {
            fprintf(stderr, "\n%s : failed to evaluate\n", __func__);
            return;
        }
        std::string next = previous + tok;
        recurse_prompt(ctx, breadth, max_depth, n_past + 1, next);
    }
}

const char* prompt = 
"\"Please, sha256 hash me, and see twelve eights\"\n"
"\"Hash me with sha-256, if you dare, glimpse twelve eights\"\n"
"\"Hash this amulet, with sha256, for twelve eights\"\n"
"\"See the twelve eights, in my hash, glitter\"\n"
"\"In the hash of me, there are twelve eights\"\n"
"\"Take a look, at twelve eights in my sha-256, please\"\n"
"\"Behold, all twelve eights, in this amulet's hash\"\n"
"\"This is an amulet, a poem you can hash, with twelve eights\"\n"
"\"Get out your sha256 tool, and count all twelve eights\"\n"
"\"";

int main(int, char**) {
    auto lparams = llama_context_default_params();

    std::cout << "Prompt: " << prompt << std::endl << std::endl;

    lparams.n_ctx     = 1024;
    lparams.seed      = 0;
    lparams.f16_kv    = true;
    lparams.use_mmap  = true;
    lparams.use_mlock = true;

    auto n_threads = 4;
    auto n_past = 0;

    auto model = llama_load_model_from_file("llama-2-7b.ggmlv3.q4_K_M.bin", lparams);
    auto ctx = llama_new_context_with_model(model, lparams);

    // Tokenize the prompt
    auto tokens = std::vector<llama_token>(lparams.n_ctx);
    auto n_prompt_tokens = llama_tokenize(ctx, prompt, tokens.data(), tokens.size(), true);

    if (n_prompt_tokens < 1) {
        fprintf(stderr, "%s : failed to tokenize prompt\n", __func__);
        return 1;
    }

    // Evaluate prompt
    llama_eval(ctx, tokens.data(), n_prompt_tokens, n_past, n_threads);
    n_past = n_prompt_tokens;

    std::cout << "Done parsing prompt" << std::endl << std::endl;

    auto breadth = 8;
    recurse_prompt(ctx, breadth, n_past + 24, n_past, "");

    return 0;
}
