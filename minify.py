import os
import re

def minify_css(content):
    # Remove comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove whitespace
    content = re.sub(r'\s+', ' ', content)
    content = re.sub(r'\s*([\{\};:,])\s*', r'\1', content)
    return content.strip()

def minify_js(content):
    # Remove single line comments
    content = re.sub(r'//.*', '', content)
    # Remove multi-line comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove whitespace
    content = re.sub(r'\s+', ' ', content)
    content = re.sub(r'\s*([\{\};:,=])\s*', r'\1', content)
    return content.strip()

def minify_html(content):
    # Remove comments
    content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)
    # Remove whitespace between tags
    content = re.sub(r'>\s+<', '><', content)
    # Remove extra whitespace
    content = re.sub(r'\s+', ' ', content)
    return content.strip()

def process_file(filepath):
    print(f"Processing: {filepath}")
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    ext = os.path.splitext(filepath)[1]
    
    if ext == '.css':
        content = minify_css(content)
    elif ext == '.js':
        content = minify_js(content)
    elif ext == '.html':
        content = minify_html(content)
    else:
        return

    return content

def main():
    source_dir = 'data/ota'
    output_dir = 'data/ota_min' # Safety first: write to new dir

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for filename in os.listdir(source_dir):
        file_path = os.path.join(source_dir, filename)
        if os.path.isfile(file_path):
            minified_content = process_file(file_path)
            if minified_content:
                out_path = os.path.join(output_dir, filename)
                with open(out_path, 'w', encoding='utf-8') as f:
                    f.write(minified_content)
                print(f"Saved to: {out_path}")

    print("\nMinification Complete! Check 'data/ota_min' folder.")
    print("If satisfied, you can replace the files in 'data/ota' with these.")

if __name__ == '__main__':
    main()
