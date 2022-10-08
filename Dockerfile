FROM arczipt/di-proxy-builder

WORKDIR /app
COPY . /app/proxy

WORKDIR /app/proxy/build
RUN cmake ..
RUN cmake --build .

CMD ["./proxy"]