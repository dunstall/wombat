FROM ubuntu

COPY . .

ARG ZOOKEEPER_ID
RUN mkdir -p /var/lib/zookeeper && \
    echo $ZOOKEEPER_ID > /var/lib/zookeeper/myid && \
    ./install.sh $ZOOKEEPER_ID
CMD JAVA_HOME=/usr/local/jdk1.8.0_241 /usr/local/zookeeper/bin/zkServer.sh start-foreground
