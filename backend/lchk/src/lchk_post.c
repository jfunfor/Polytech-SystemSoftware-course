#include "lchk.h"

int lchk_send_result(const char *json_payload, const char *url) {
    CURL *curl;
    CURLcode res;
    int attempt = 0;
    int success = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "[ERROR] Failed to init CURL\n");
        return 0;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    while (attempt < LCHK_MAX_RETRIES) {
        if (attempt > 0) {
            LOG("Retry attempt %d...\n", attempt + 1);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); /* 10s timeout */

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "[ERROR] Failed to send result: %s\n", curl_easy_strerror(res));
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code >= 200 && response_code < 300) {
                LOG("Successfully sent result (HTTP %ld)\n", response_code);
                success = 1;
                break;
            } else {
                fprintf(stderr, "[ERROR] HTTP response code %ld\n", response_code);
            }
        }

        attempt++;
        if (!success && attempt < LCHK_MAX_RETRIES) {
            sleep(1 << attempt); /* 2^attempt seconds */
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (!success) {
        fprintf(stderr, "[ERROR] Failed after %d attempts\n", LCHK_MAX_RETRIES);
    }

    return success;
}
