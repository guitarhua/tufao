/*  This file is part of the Tufão project
    Copyright (C) 2012 Vinícius dos Santos Oliveira <vini.ipsmaker@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any
    later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TUFAO_WEBSOCKET_H
#define TUFAO_WEBSOCKET_H

#include "abstractmessagesocket.h"
#include "headers.h"

class QHostAddress;

namespace Tufao {
namespace Priv {

struct WebSocket;
union Frame;

} // namespace Priv

class HttpServerRequest;

class TUFAO_EXPORT WebSocket : public AbstractMessageSocket
{
    Q_OBJECT
public:
    enum MessageType
    {
        TEXT_MESSAGE,
        BINARY_MESSAGE
    };

    explicit WebSocket(QObject *parent = 0);
    ~WebSocket();

    /*!
      \note
      Optional headers such as Origin aren't added by default. If you want to
      use one of the headers, you should add them explicitly. The list of
      optional headers is the following:
        - Origin: RFC 6454. If your software is a browser client, this field is
          mandatory, not optional.
        - Sec-WebSocket-Protocol: If present, this value indicates one or more
          comma-separated subprotocol the client wishes to speak, ordered by
          preference.

      \note
      If the object fail to establish a connection, it will emit disconnected
      signal. If it procceds, it will emit the connected() signal.
      */
    bool startClientHandshake(const QHostAddress &address, quint16 port,
                              const QByteArray &resource,
                              const Headers &headers = Headers());

    /*!
      This is an overloaded function.

      It uses port 80 to establish the connection.
      */
    bool startClientHandshake(const QHostAddress &address,
                              const QByteArray &resource,
                              const Headers &headers = Headers());

    /*!
      \note
      You should call this function only after \p request emitts the
      Tufao::HttpServerRequest::upgrade signal.

      \note
      WebSocketNode hold any info about the request. If you need info, get
      yourself. A list of interesting headers may be:
        - Origin: Compare the Origin to the Host to block malicious scripts
          coming from web browsers.
        - Cookie
        - Sec-WebSocket-Protocol

      \note
      If you want to perform additional client authentication, you should
      start the handshake only after the authentication occurs. You can use the
      401 status code with "WWW-Authenticate" header to perform the
      authentication.

      \note
      You can use a 3xx status code to redirect the client.

      \note
      This method may write to the socket and fail to create a WebSocket
      connection, returning false, but it won't close the connection.

      \param headers extra headers that you can send to the client, such as
      "Sec-WebSocket-Protocol" to specify which subprotocol to use
      */
    bool startServerHandshake(const HttpServerRequest *request,
                              const QByteArray &head,
                              const Headers &headers = Headers());

    /*!
      Set the type of messages sent through WebSocket::sendMessage method.

      \note
      Another way of choose the type of sent messages is through
      WebSocket::sendBinaryMessage and WebSocket::sendUtf8Message methods.

      \note
      The default value is BINARY_MESSAGE
      */
    void setMessagesType(MessageType type);

    /*!
      Return the current type of messages what will be sent through
      WebSocket::sendMessage method.

      \sa
      Tufao::WebSocket::setMessagesType
      */
    MessageType messagesType();

    static QList<QByteArray> supportedProtocols(const Headers &headers);

signals:
    void pong(QByteArray data);

public slots:
    void close();
    bool sendMessage(const QByteArray &msg);
    bool sendBinaryMessage(const QByteArray &msg);
    bool sendUtf8Message(const QByteArray &msg);
    bool ping(const QByteArray &data);

private slots:
    void onOpenningError();
    void onConnected();
    void onReadyRead();
    void onDisconnected();

private:
    bool isResponseOkay();

    Priv::Frame standardFrame() const;
    Priv::Frame controlFrame() const;

    void writePayload(Priv::Frame frame, const QByteArray &data);
    void close(quint16 code);

    void readData(const QByteArray &data);
    void parseBuffer();
    bool parseFrame();
    bool parseSize16();
    bool parseSize64();
    bool parseMaskingKey();
    bool parsePayloadData();

    void decodeFragment(QByteArray &fragment);
    void evaluateControlFrame();

    Priv::WebSocket *priv;
};

} // namespace Tufao

#endif // TUFAO_WEBSOCKET_H