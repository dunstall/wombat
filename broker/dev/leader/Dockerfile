FROM gcc:latest

WORKDIR /usr/local/wombat

COPY . .
            
WORKDIR /usr/local/wombat/log
RUN make install && make -j 4

WORKDIR /usr/local/wombat/partition
RUN make install && make -j 4

WORKDIR /usr/local/wombat/broker
RUN make install && make -j 4

CMD ./broker leader
