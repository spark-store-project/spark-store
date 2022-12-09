TARGET = spark-dstore-patch
TEMPLATE += app
QT +=  widgets
SOURCES += spark-dstore-tool.cpp

# Default rules for deployment.
qnx: target.path = /tmp/spark-store/bin
else: unix:!android: target.path = /opt/durapps/spark-store/bin
!isEmpty(target.path): INSTALLS += target