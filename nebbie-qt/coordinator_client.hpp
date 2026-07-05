#pragma once

#include "nebbie/world_index.hpp"

#include <QString>
#include <optional>
#include <vector>

class QNetworkAccessManager;

namespace nebbie::qt {

struct CoordinatorFetchResult {
    bool ok = false;
    QString error;
    std::optional<nebbie::WorldIndex> index;
    std::vector<nebbie::WorldIndexReservation> reservations;
};

CoordinatorFetchResult fetch_coordinator_sync(QNetworkAccessManager& network,
                                              const QString& index_url,
                                              const QString& coordinator_url,
                                              const QString& coordinator_token,
                                              bool fetch_index = true);

bool post_reservation_sync(QNetworkAccessManager& network,
                           const QString& coordinator_url,
                           const QString& coordinator_token,
                           const nebbie::WorldIndexReservation& reservation,
                           QString* error_out = nullptr);

} // namespace nebbie::qt
