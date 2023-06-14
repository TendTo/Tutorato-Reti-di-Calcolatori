#!/bin/bash
#================================================================
# HEADER
#================================================================
#% SYNOPSIS
#+    ${SCRIPT_NAME} [OPTIONS]
#%
#% DESCRIPTION
#%    Copy the vendor/index.html file for each folder inside 'lezioni'.
#%    This will allow the creation of a static website.
#%
#% OPTIONS
#%    -d, --directory         Path to the directory that contains the 'lezioni' subdirectories.
#%                            If set to an empty string, the current directory will be used.
#%                            Defaults to 'lezioni'
#%    -e, --exercise          Path to the directory that contains the 'esercizi' subdirectories.
#%                            If set to an empty string, the current directory will be used.
#%                            Defaults to 'esercizi'
#%    -m, --markdown          Path to the markdown file to look for in each folder.
#%                            Defaults to 'README.md'.
#%    -c, --clean             Remove all the generated html files.
#%    -h, --help              Print this help
#%    -v, --version           Print script information
#%
#% EXAMPLES
#%    ${SCRIPT_NAME}
#%    ${SCRIPT_NAME} -c
#%    ${SCRIPT_NAME} -m "README.md"
#%    ${SCRIPT_NAME} -d ./lezioni -m "FIle.md"
#%
#================================================================
#- IMPLEMENTATION
#-    version         ${SCRIPT_NAME} 0.0.1
#-    author          TendTo
#-    copyright       Copyright (c) https://github.com/TendTo
#-    license         GNU General Public License
#-
#================================================================
# END_OF_HEADER
#================================================================


# DESC: Usage help and version info
# ARGS: None
# OUTS: None
# NOTE: Used to document the usage of the script
#       and to display its version when requested or
#       if some arguments are not valid
usage() { printf "Usage: "; head -${script_headsize:-99} ${0} | grep -e "^#+" | sed -e "s/^#+[ ]*//g" -e "s/\${SCRIPT_NAME}/${script_name}/g" ; }
usagefull() { head -${script_headsize:-99} ${0} | grep -e "^#[%+-]" | sed -e "s/^#[%+-]//g" -e "s/\${SCRIPT_NAME}/${script_name}/g" ; }
scriptinfo() { head -${script_headsize:-99} ${0} | grep -e "^#-" | sed -e "s/^#-//g" -e "s/\${SCRIPT_NAME}/${script_name}/g"; }

# DESC: Generic script initialisation
# ARGS: $@ (optional): Arguments provided to the script
# OUTS: $orig_cwd: The current working directory when the script was run
#       $script_path: The full path to the script
#       $script_dir: The directory path of the script
#       $script_name: The file name of the script
#       $script_params: The original parameters provided to the script
#       $ta_none: The ANSI control code to reset all text attributes
# NOTE: $script_path only contains the path that was used to call the script
#       and will not resolve any symlinks which may be present in the path.
#       You can use a tool like realpath to obtain the "true" path. The same
#       caveat applies to both the $script_dir and $script_name variables.
function script_init() {
    # Useful variables
    readonly orig_cwd="$PWD"
    readonly script_params="$*"
    readonly script_path="${BASH_SOURCE[0]}"
    script_dir="$(dirname "$script_path")"
    script_name="$(basename "$script_path")"
    readonly script_dir script_name
    readonly root_dir="$script_dir"
    readonly ta_none="$(tput sgr0 2> /dev/null || true)"
    readonly script_headsize=$(head -200 ${0} |grep -n "^# END_OF_HEADER" | cut -f1 -d:)
}

# DESC: Parameter parser
# ARGS: $@ (optional): Arguments provided to the script
# OUTS: Variables indicating command-line parameters and options
function parse_args
{
    # Local variable
    local param
    # Positional args
    args=()

    # Optional args
    dir="lezioni"
    markdown="README.md"
    exercise="esercizi"

    # Named args
    while [ $# -gt 0 ]; do
        param="$1"
        shift
        case "$param" in
            -h )
                usage
                exit 0
            ;;
            --help )
                usagefull
                exit 0
            ;;
            -v | --version )
                scriptinfo
                exit 0
            ;;
            -c | --clean )
                clean_flag=true
            ;;
            -d | --directory )
                dir="$1"
                shift
            ;;
            -e | --exercise )
                exercise="$1"
                shift
            ;;
            -m | --markdown )
                markdown="$1"
                shift
            ;;
            * )
                args+=("$param")
            ;;
        esac
    done

    # Restore positional args
    set -- "${args[@]}"

}

function clean
{
    find "$dir" -name "*.html" -type f -delete
    find "$dir" -name "*.class" -type f -delete
    find "$exercise" -type f -executable -not -name "*.sh" -not -name "*.bat" -delete
    rm -f "$root_dir/index.html"
}

function create_slides
{
    for folder in "$dir"/*; do
        if [[ -d "$folder" ]]; then
            folder=$(basename "$folder")
            html_file="$root_dir/$dir/${folder}/index.html"
            cp "$root_dir/vendor/index.html" "$html_file"
            sed -i "s|\"\./|\"../../vendor/|g" "$html_file"
            sed -i "s|data-markdown=\"README.md\"|data-markdown=\"./$markdown\"|g" "$html_file"
            sed -i "s|{{TITLE}}|${folder}|g" "$html_file"
            if ! grep -q "\`\`\`mermaid" "$root_dir/$dir/${folder}/$markdown"; then
                sed -i "/mermaid-load\.js/d" "$html_file"
            fi
        fi
    done
}

function create_index
{
    html_file="$root_dir/index.html"
    cp $root_dir/vendor/index.html "$html_file"
    sed -i "s|\"\./|\"./vendor/|g" "$html_file"
    sed -i "s|{{TITLE}}|Tutorato di Reti 2022-20223|g" "$html_file"
    sed -i "s|data-markdown=\"README.md\"|data-markdown=\"./$markdown\"|g" "$html_file"
    if ! grep -q "\`\`\`mermaid" "$root_dir/$markdown"; then
        sed -i "/mermaid\.min\.js/d" "$html_file"
    fi
}

# DESC: Main control flow
# ARGS: $@ (optional): Arguments provided to the script
# OUTS: None
function main() {
    script_init "$@"
    parse_args "$@"

    if [[ "$clean_flag" == true ]]; then
        clean
        exit 0
    fi

    create_slides
    create_index
}

# Invoke main with args if not sourced
if ! (return 0 2> /dev/null); then
    main "$@"
fi