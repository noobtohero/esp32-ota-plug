import os
import gzip
import shutil

DATA_DIR = 'data'

def compress_files():
    if not os.path.exists(DATA_DIR):
        print(f"Directory '{DATA_DIR}' not found.")
        return

    print("Compressing web assets...")
    for root, dirs, files in os.walk(DATA_DIR):
        for file in files:
            if file.endswith(('.html', '.css', '.js', '.json', '.xml', '.svg')):
                file_path = os.path.join(root, file)
                gz_path = file_path + '.gz'
                
                # Check if compression needed (source newer than gz)
                if not os.path.exists(gz_path) or os.path.getmtime(file_path) > os.path.getmtime(gz_path):
                    with open(file_path, 'rb') as f_in:
                        with gzip.open(gz_path, 'wb') as f_out:
                            shutil.copyfileobj(f_in, f_out)
                    print(f"[GZ] {file} -> {file}.gz")
                else:
                    print(f"[OK] {file}")

if __name__ == '__main__':
    compress_files()
