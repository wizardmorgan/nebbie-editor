#include "coordinator_client.hpp"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace nebbie::qt {

namespace {

QByteArray wait_for_reply(QNetworkAccessManager& network, QNetworkReply* reply, QString* error_out) {
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        if (error_out) {
            *error_out = reply->errorString();
        }
        return {};
    }

    return reply->readAll();
}

QNetworkRequest make_request(const QUrl& url, const QString& token = {}) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("NebbieEditor/1.0"));
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + token.toUtf8());
    }
    return request;
}

} // namespace

CoordinatorFetchResult fetch_coordinator_sync(QNetworkAccessManager& network,
                                              const QString& index_url,
                                              const QString& coordinator_url,
                                              const QString& coordinator_token,
                                              const bool fetch_index) {
    CoordinatorFetchResult result;

    if (fetch_index) {
        if (index_url.isEmpty()) {
            result.error = QStringLiteral("index_url non configurato");
            return result;
        }

        QString fetch_error;
        QNetworkReply* index_reply = network.get(make_request(QUrl(index_url)));
        const QByteArray index_body = wait_for_reply(network, index_reply, &fetch_error);
        index_reply->deleteLater();
        if (index_body.isEmpty()) {
            result.error = fetch_error.isEmpty() ? QStringLiteral("Risposta indice vuota") : fetch_error;
            return result;
        }

        const auto parsed_index = nebbie::world_index_from_json(index_body.toStdString());
        if (!parsed_index) {
            result.error = QStringLiteral("JSON indice mondo non valido");
            return result;
        }
        result.index = *parsed_index;
    }

    if (!coordinator_url.isEmpty() && !coordinator_token.isEmpty()) {
        QString fetch_error;
        const QUrl reservations_url(coordinator_url.endsWith('/')
                                        ? coordinator_url + QStringLiteral("reservations")
                                        : coordinator_url + QStringLiteral("/reservations"));
        QNetworkReply* reservations_reply =
            network.get(make_request(reservations_url, coordinator_token));
        const QByteArray reservations_body = wait_for_reply(network, reservations_reply, &fetch_error);
        reservations_reply->deleteLater();
        if (!reservations_body.isEmpty()) {
            const auto parsed_reservations =
                nebbie::coordinator_reservations_from_json(reservations_body.toStdString());
            if (parsed_reservations) {
                result.reservations = *parsed_reservations;
                if (result.index) {
                    nebbie::merge_reservations(*result.index, result.reservations);
                }
            }
        } else if (!fetch_index) {
            result.error = fetch_error.isEmpty() ? QStringLiteral("Risposta prenotazioni vuota") : fetch_error;
            return result;
        }
    }

    if (fetch_index && !result.index) {
        result.error = QStringLiteral("Indice mondo non disponibile");
        return result;
    }

    result.ok = true;
    return result;
}

bool post_reservation_sync(QNetworkAccessManager& network,
                           const QString& coordinator_url,
                           const QString& coordinator_token,
                           const nebbie::WorldIndexReservation& reservation,
                           QString* error_out) {
    if (coordinator_url.isEmpty() || coordinator_token.isEmpty()) {
        if (error_out) {
            *error_out = QStringLiteral("coordinator_url o coordinator_token non configurati");
        }
        return false;
    }

    QJsonObject body;
    body.insert(QStringLiteral("builder"), QString::fromStdString(reservation.builder));
    body.insert(QStringLiteral("zone_num"), reservation.zone_num);
    body.insert(QStringLiteral("kind"), QString::fromStdString(reservation.kind));
    body.insert(QStringLiteral("start_vnum"), static_cast<qint64>(reservation.start_vnum));
    body.insert(QStringLiteral("end_vnum"), static_cast<qint64>(reservation.end_vnum));
    body.insert(QStringLiteral("note"), QString::fromStdString(reservation.note));
    if (!reservation.expires_at.empty()) {
        body.insert(QStringLiteral("expires_at"), QString::fromStdString(reservation.expires_at));
    }

    const QUrl reservations_url(coordinator_url.endsWith('/')
                                    ? coordinator_url + QStringLiteral("reservations")
                                    : coordinator_url + QStringLiteral("/reservations"));
    QNetworkRequest request = make_request(reservations_url, coordinator_token);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply* reply =
        network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    const QByteArray response_body = wait_for_reply(network, reply, error_out);
    const bool ok = !response_body.isEmpty();
    reply->deleteLater();
    return ok;
}

} // namespace nebbie::qt
