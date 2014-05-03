# exec tmux if installed and has a configuration
test -z "${TMUX}" && which tmux > /dev/null 2>&1 && test -f "${HOME}/.tmux.conf" && exec tmux att
