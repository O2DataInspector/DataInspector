FROM di-proxy-builder

WORKDIR /app
COPY . /app/proxy

RUN echo "export ALIBUILD_WORK_DIR=/di/alice-sw" >> .bashrc
RUN echo "eval \"`alienv shell-helper`\"" >> .bashrc

WORKDIR /app/proxy/build
RUN cmake ..
RUN cmake --build .

CMD ["./proxy", "../scripts/executeWorkflow", "../o2-local-builds"]