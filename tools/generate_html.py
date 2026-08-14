from pathlib import Path
import os

output_file = Path("src/web/page.h")

output = ""

output_end = '''
void init_pages(httpd_handle_t server) {
    httpd_uri_t uri_root = {};

'''

pageCount = 0;

for subdir, dirs, files in os.walk(Path("web")):
    for file in files:
        print(os.path.join(subdir, file))
        webpath = (subdir + "/" + file).replace("\\", "/").replace("web/", "")
        cpath = "file_" + webpath.replace("/", "_").replace(".", "_")
        data = Path(os.path.join(subdir, file)).read_text(encoding="utf-8")

        t = file[file.index('.') + 1 :].replace("js", "javascript")
        pageCount += 1

        output += f'''
static const char* {cpath} = R"raw(
{data}
)raw";
static esp_err_t page_get_{cpath}_handler(httpd_req_t *req) {{
    httpd_resp_set_type(req, "text/{t}; charset=UTF-8");

    httpd_resp_send(
        req,
        {cpath},
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}}
'''

        output_end += f'''
    uri_root.uri = "/{webpath}";
    uri_root.method = HTTP_GET;
    uri_root.handler = page_get_{cpath}_handler;
    uri_root.user_ctx = nullptr;

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(
            server,
            &uri_root
        )
    );
    '''




output_file.write_text(f'''
// Generated with tools/generate_html.py
#pragma once
#define PAGE_COUNT {pageCount}
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"
''' + output + output_end + "}", encoding="utf-8")

print(f"Generated: {output_file}")
