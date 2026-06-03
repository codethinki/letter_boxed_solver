import sys
import os

def convert_to_lowercase(input_path, output_path=None):
    if not output_path:
        output_path = input_path

    try:
        # Read the file
        with open(input_path, 'r', encoding='utf-8', errors='ignore') as file:
            content = file.read()
        
        # Convert to lowercase (only affects uppercase letters)
        lowercased_content = content.lower()
        
        # Save the file
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(lowercased_content)
            
        print(f"Success! Saved to: {output_path}")
        
    except FileNotFoundError:
        print(f"Error: The file '{input_path}' could not be found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    # If file paths are provided as command-line arguments
    if len(sys.argv) > 1:
        infile = sys.argv[1]
        outfile = sys.argv[2] if len(sys.argv) > 2 else None
        convert_to_lowercase(infile, outfile)
    else:
        # Interactive prompts if run without arguments
        infile = input("Enter the path to the input file: ").strip('" ')
        overwrite = input("Overwrite the original file? (y/n): ").strip().lower()
        
        if overwrite == 'y':
            convert_to_lowercase(infile)
        else:
            outfile = input("Enter the path for the new output file: ").strip('" ')
            convert_to_lowercase(infile, outfile)