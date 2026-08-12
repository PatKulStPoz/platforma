from pathlib import Path

html_file = Path("web/page.html")
output_file = Path("src/page.h")

html = html_file.read_text(encoding="utf-8")

output = f'''#pragma once

static const char *html_page = R"raw(
{html}
)raw";
'''

output_file.write_text(output, encoding="utf-8")

print(f"Generated: {output_file}")