if [[ $(uname -s) == Linux ]]; then
    :
elif [[ $(uname -s) == Darwin ]]; then
    OPENSSL_DIR=/usr/local/opt/openssl
    PATH="$OPENSSL_DIR/bin:$PATH"
    LDFLAGS="$LDFLAGS -L${OPENSSL_DIR}/lib"
    CFLAGS="$CFLAGS -I${OPENSSL_DIR}/include"

    GTAR_DIR=/usr/local/opt/gnu-tar
    PATH="$GTAR_DIR/libexec/gnubin:$PATH"

    export PATH
    export LDFLAGS
    export CFLAGS
fi
