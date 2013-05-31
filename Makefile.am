bin_PROGRAMS = \
    fakeGps \
    trafficConsumer \
    trafficProducer \
    photoConsumer \
    photoProducer

sbin_PROGRAMS = ndnd

noinst_LIBRARIES = \
    libccnbparser.a \
    libndncore.a \
    libndnclient.a \
    libndngeo.a

EXTRA_DIST = bootstrap Doxyfile LICENSE README.md

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -DNDN_LOG_ENABLE $(BOOST_CPPFLAGS)
AM_CXXFLAGS = -Wall $(LIBEVENT_CFLAGS) $(YAJL_CFLAGS)
AM_LDFLAGS = -Wl,-O1 $(BOOST_THREAD_LDFLAGS)

LDADD = libndncore.a libccnbparser.a $(BOOST_THREAD_LIBS) $(LIBEVENT_LIBS) $(YAJL_LIBS) -lrt

libccnbparser_a_SOURCES = \
    helper/ccnb-parser/ccnb-parser-common.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-attr.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-attr.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-base-attr.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-base-tag.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-blob.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-blob.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-block.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-block.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-dattr.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-dattr.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-dtag.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-dtag.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-ext.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-ext.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-tag.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-tag.h \
    helper/ccnb-parser/syntax-tree/ccnb-parser-udata.cc \
    helper/ccnb-parser/syntax-tree/ccnb-parser-udata.h \
    helper/ccnb-parser/visitors/ccnb-parser-content-object-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-content-object-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-depth-first-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-depth-first-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-interest-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-interest-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-name-components-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-name-components-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-no-argu-depth-first-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-no-argu-depth-first-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-no-argu-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-non-negative-integer-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-non-negative-integer-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-nonce-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-nonce-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-string-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-string-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-timestamp-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-timestamp-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-void-depth-first-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-void-depth-first-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-void-no-argu-depth-first-visitor.cc \
    helper/ccnb-parser/visitors/ccnb-parser-void-no-argu-depth-first-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-void-no-argu-visitor.h \
    helper/ccnb-parser/visitors/ccnb-parser-void-visitor.h

libndncore_a_SOURCES = \
    corelib/assert.h \
    corelib/default-deleter.h \
    corelib/empty.h \
    corelib/fatal-error.h \
    corelib/log.cc \
    corelib/log.h \
    corelib/pretty-print.h \
    corelib/ptr.h \
    corelib/simple-ref-count.h \
    corelib/singleton.h \
    network/buffer.cc \
    network/buffer.h \
    network/header.h \
    network/ndn-content-object-header.cc \
    network/ndn-content-object-header.h \
    network/ndn-content-packet.cc \
    network/ndn-content-packet.h \
    network/ndn-interest-header.cc \
    network/ndn-interest-header.h \
    network/ndn-name-components.cc \
    network/ndn-name-components.h \
    helper/event-monitor.cc \
    helper/event-monitor.h \
    helper/monitorable.h \
    helper/ndn-header-helper.cc \
    helper/ndn-header-helper.h \
    helper/ndn-decoding-helper.cc \
    helper/ndn-decoding-helper.h \
    helper/ndn-encoding-helper.cc \
    helper/ndn-encoding-helper.h \
    network/packet.cc \
    network/packet.h

libndnclient_a_SOURCES = \
    apps/application-map.cc \
    apps/application-map.h \
    apps/ndn-app-socket.cc \
    apps/ndn-app-socket.h \
    apps/ndn-consumer.cc \
    apps/ndn-consumer.h \
    apps/ndn-producer.cc \
    apps/ndn-producer.h

libndngeo_a_SOURCES = \
    utils/geo/coordinate.cc \
    utils/geo/coordinate.h \
    utils/geo/gps-info.cc \
    utils/geo/gps-info.h \
    utils/geo/gpsd-parser.cc \
    utils/geo/gpsd-parser.h \
    utils/geo/junction.cc \
    utils/geo/junction.h \
    utils/geo/link.cc \
    utils/geo/link.h \
    utils/geo/location-service.cc \
    utils/geo/location-service.h \
    utils/geo/map-element.cc \
    utils/geo/map-element.h \
    utils/geo/map.cc \
    utils/geo/map.h \
    utils/gpsd-util.cc \
    utils/gpsd-util.h \
    network/mac/ll-header.h

fakeGps_CPPFLAGS =
fakeGps_CXXFLAGS = -Wall
fakeGps_LDFLAGS = -Wl,-O1
fakeGps_LDADD =
fakeGps_SOURCES = utils/fake-gps.cc

trafficConsumer_LDADD = libndnclient.a libndngeo.a $(LDADD)
trafficConsumer_SOURCES = \
    apps/traffic-consumer.cc \
    apps/traffic-app.h

trafficProducer_LDADD = libndnclient.a libndngeo.a $(LDADD)
trafficProducer_SOURCES = \
    apps/traffic-producer.cc \
    apps/traffic-app.h

photoConsumer_LDADD = libndnclient.a libndngeo.a $(LDADD)
photoConsumer_SOURCES = \
    apps/photo-consumer.cc \
    apps/photo-app.h

photoProducer_LDADD = libndnclient.a libndngeo.a $(LDADD)
photoProducer_SOURCES = \
    apps/photo-producer.cc \
    apps/photo-app.h

ndnd_LDADD = libndngeo.a $(LDADD)
ndnd_SOURCES = \
    daemon/app-connector.cc \
    daemon/app-connector.h \
    daemon/hash-helper.h \
    daemon/ndn-management.cc \
    daemon/ndn-management.h \
    daemon/ndn-adhoc-net-device-face.cc \
    daemon/ndn-adhoc-net-device-face.h \
    daemon/ndn-local-face.cc \
    daemon/ndn-local-face.h \
    daemon/ndn-l3-protocol.cc \
    daemon/ndn-l3-protocol.h \
    daemon/cs/ndn-content-store.h \
    daemon/cs/ndn-content-store.cc \
    daemon/cs/content-store-impl.h \
    daemon/ndn-face.cc \
    daemon/ndn-face.h \
    daemon/ndn-fib.cc \
    daemon/ndn-fib.h \
    daemon/ndn-flooding-strategy.cc \
    daemon/ndn-flooding-strategy.h \
    daemon/ndn-forwarding-strategy.cc \
    daemon/ndn-forwarding-strategy.h \
    daemon/ndn-net-device-face.cc \
    daemon/ndn-hub-over-ip-device-face.h \
    daemon/ndn-hub-over-ip-device-face.cc \
    daemon/ndn-net-device-face.h \
    daemon/pit/ndn-pit.cc \
    daemon/pit/ndn-pit.h \
    daemon/pit/ndn-pit-entry-incoming-face.cc \
    daemon/pit/ndn-pit-entry-incoming-face.h \
    daemon/pit/ndn-pit-entry-outgoing-face.cc \
    daemon/pit/ndn-pit-entry-outgoing-face.h \
    daemon/pit/ndn-pit-entry.cc \
    daemon/pit/ndn-pit-entry.h \
    daemon/ndn.h \
    daemon/ndnd.cc \
    network/mac/ndn-device-adapter.cc \
    network/mac/ndn-device-adapter.h \
    network/mac/ndnsock/ndn-raw-socket.cc \
    network/mac/ndnsock/ndn-raw-socket.h \
    network/mac/ndnsock/ndn-socket.h \
    network/mac/ndnsock/ndn-socket-exception.h \
    network/mac/ack-info.cc \
    network/mac/ack-info.h \
    network/mac/ack-info-by-coordinate.h \
    network/mac/link-layer.h \
    network/mac/ll-device.cc \
    network/mac/ll-device.h \
    network/mac/ll-nom-policy.cc \
    network/mac/ll-nom-policy.h \
    network/mac/ll-policy.cc \
    network/mac/ll-policy.h \
    network/mac/ll-upper-layer-comm-service.cc \
    network/mac/ll-upper-layer-comm-service.h \
    network/mac/ll-metadata.cc \
    network/mac/ll-metadata.h \
    network/mac/ll-metadata-80211-adhoc.cc \
    network/mac/ll-metadata-80211-adhoc.h \
    network/mac/ll-metadata-over-ip.cc \
    network/mac/ll-metadata-over-ip.h \
    network/mac/lal-ack-manager.cc \
    network/mac/lal-ack-manager.h \
    network/mac/lal-ack-manager-by-distance.cc \
    network/mac/lal-ack-manager-by-distance.h \
    network/mac/lal-stats.cc \
    network/mac/lal-stats.h \
    network/mac/geo-storage.cc \
    network/mac/geo-storage.h \
    network/mac/packet-storage.cc \
    network/mac/packet-storage.h \
    network/request-source-info.h \
    network/request-source-ip-info.cc \
    network/request-source-ip-info.h \
    utils/lru-policy.h \
    utils/trie.h \
    utils/trie-with-policy.h
