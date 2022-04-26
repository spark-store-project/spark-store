FROM shenmo7192/uos-21-dtk5.4:1.0
ADD . /root/workdir
WORKDIR /root/workdir
RUN dpkg-buildpackage
RUN mv *.deb /root
RUN rm -rf /root/workdir
RUN mv /root/*.deb /root/workdir
