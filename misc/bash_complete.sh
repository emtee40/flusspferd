#
# bash completion support for flusspferd.
#
# Copyright (C) 2009 Aristid Breitkreuz, Ash Berlin, Rüdiger Sonderfeld
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

_flusspferd() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="-h --help -v --version -c --config -i --interactive -0 --machine-mode -e --expression -f --file -I --include-path -M --module -m --main --no-global-history --history-file --"

    if [[ ${cur} == -* ]] ; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
    else
        case "$prev" in
            -c|--config|-f|--file|--history-file)
                COMPREPLY=( $(compgen -f ${cur}) )
                return 0
                ;;
            -I|--include-path)
                COMPREPLY=( $(compgen -d ${cur}) )
                return 0
                ;;
            -e|--expression)
                return 0
                ;;
            -m|-M|--module|--main)
                ## TODO module completion
                return 0
                ;;
            *)
                COMPREPLY=( $(compgen -f ${cur}) )
                return 0
                ;;
        esac
    fi
}

complete -F _flusspferd flusspferd
