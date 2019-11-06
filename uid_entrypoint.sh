#!/usr/bin/env bash

export PATH=$PATH:/usr/local/bin

# Create passwd entry for arbitrary user ID
if [[ -z "$(awk -F ':' "\$3 == $(id -u)" /etc/passwd)" ]]; then
    echo "Adding arbitrary user"
    echo "${USER_NAME:-default}:x:$(id -u):0:${USER_NAME:-default} user:${HOME}:/sbin/nologin" >> /etc/passwd
    echo "$(awk -F ':' "\$3 == $(id -u)" /etc/passwd)"
fi

echo "Running as $(id -un):$(id -gn)"

exec "$@"
