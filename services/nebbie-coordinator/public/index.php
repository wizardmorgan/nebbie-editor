<?php
declare(strict_types=1);

$configPath = __DIR__ . '/../config.php';
if (!file_exists($configPath)) {
    http_response_code(500);
    header('Content-Type: application/json');
    echo json_encode(['status' => 'KO', 'reason' => 'Missing config.php']);
    exit;
}

$config = require $configPath;

function respond(int $code, array $payload): void {
    http_response_code($code);
    header('Content-Type: application/json');
    echo json_encode($payload, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
    exit;
}

function auth_ok(array $config): bool {
    $headers = function_exists('getallheaders') ? getallheaders() : [];
    $auth = $headers['Authorization'] ?? $headers['authorization'] ?? '';
    if (str_starts_with($auth, 'Bearer ')) {
        $token = substr($auth, 7);
        return hash_equals($config['api_token'], $token);
    }
    return isset($_GET['token']) && hash_equals($config['api_token'], (string)$_GET['token']);
}

function db(array $config): PDO {
    $dir = dirname($config['db_path']);
    if (!is_dir($dir)) {
        mkdir($dir, 0775, true);
    }
    $pdo = new PDO('sqlite:' . $config['db_path']);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $schema = file_get_contents(__DIR__ . '/../schema.sql');
    if ($schema !== false) {
        $pdo->exec($schema);
    }
    return $pdo;
}

$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$path = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH) ?: '/';
$path = preg_replace('#/+#', '/', $path);

if (str_ends_with($path, '/world-index.json')) {
  $file = $config['world_index_path'];
  if (!is_readable($file)) {
    respond(404, ['status' => 'KO', 'reason' => 'world-index.json not found']);
  }
  header('Content-Type: application/json');
  readfile($file);
  exit;
}

if (!auth_ok($config)) {
    respond(401, ['status' => 'KO', 'reason' => 'Unauthorized']);
}

if (str_contains($path, '/reservations')) {
    $pdo = db($config);
    if ($method === 'GET') {
        $rows = $pdo->query('SELECT id, builder, zone_num, kind, start_vnum, end_vnum, note, expires_at, status, created_at FROM reservations WHERE status = "active" ORDER BY id DESC')->fetchAll(PDO::FETCH_ASSOC);
        respond(200, ['status' => 'OK', 'reservations' => $rows]);
    }
    if ($method === 'POST') {
        $body = json_decode(file_get_contents('php://input') ?: '[]', true);
        if (!is_array($body)) {
            respond(400, ['status' => 'KO', 'reason' => 'Invalid JSON body']);
        }
        $required = ['builder', 'zone_num', 'kind', 'start_vnum', 'end_vnum'];
        foreach ($required as $field) {
            if (!isset($body[$field])) {
                respond(400, ['status' => 'KO', 'reason' => "Missing field: $field"]);
            }
        }
        if ((int)$body['end_vnum'] < (int)$body['start_vnum']) {
            respond(400, ['status' => 'KO', 'reason' => 'end_vnum must be >= start_vnum']);
        }
        $stmt = $pdo->prepare('INSERT INTO reservations (builder, zone_num, kind, start_vnum, end_vnum, note, expires_at, status) VALUES (:builder, :zone_num, :kind, :start_vnum, :end_vnum, :note, :expires_at, "active")');
        $stmt->execute([
            ':builder' => (string)$body['builder'],
            ':zone_num' => (int)$body['zone_num'],
            ':kind' => (string)$body['kind'],
            ':start_vnum' => (int)$body['start_vnum'],
            ':end_vnum' => (int)$body['end_vnum'],
            ':note' => (string)($body['note'] ?? ''),
            ':expires_at' => (string)($body['expires_at'] ?? ''),
        ]);
        respond(201, ['status' => 'OK', 'id' => (int)$pdo->lastInsertId()]);
    }
}

respond(404, ['status' => 'KO', 'reason' => 'Not found']);
