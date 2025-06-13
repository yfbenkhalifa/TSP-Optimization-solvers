import os
import subprocess
import argparse
import glob

def find_tsp_files(root_dir):
    tsp_files = []
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith('.tsp'):
                tsp_files.append(os.path.join(dirpath, filename))
    return tsp_files

def run_main_on_tsp(main_exe, tsp_file, method, log_file, extra_args=None):
    cmd = [main_exe, '-m', method, '--log', log_file, tsp_file]
    if extra_args:
        cmd.extend(extra_args)
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print(result.stderr)
    return result.returncode

def main():
    parser = argparse.ArgumentParser(description='Batch run TSP solver on all .tsp files in a directory tree.')
    parser.add_argument('main_exe', help='Path to the compiled main executable')
    parser.add_argument('root_dir', help='Root directory to search for .tsp files')
    parser.add_argument('--method', default='branch_and_cut', help='TSP solving method to use')
    parser.add_argument('--log', default='batch_log.csv', help='CSV log file to write results to')
    parser.add_argument('--extra', nargs=argparse.REMAINDER, help='Extra arguments to pass to main')
    args = parser.parse_args()

    tsp_files = find_tsp_files(args.root_dir)
    if not tsp_files:
        print(f"No .tsp files found in {args.root_dir}")
        return
    print(f"Found {len(tsp_files)} .tsp files.")

    for tsp_file in tsp_files:
        run_main_on_tsp(args.main_exe, tsp_file, args.method, args.log, args.extra)

if __name__ == '__main__':
    main()

