# Engine Server

## Vue d'ensemble

`Server` est une classe qui encapsule la classe **Server de la cpp-httplib**.

Le système de **route groups** et **middlewares** s'inspire du framework Gin (Go) et permet une organisation claire et modulaire des routes HTTP.

## Concepts clés

### Route Groups
Un groupe de routes permet de :
- **Préfixer les routes** : Tous les handlers du groupe partagent un préfixe de chemin
- **Réutiliser les middlewares** : Appliquer les mêmes middlewares à plusieurs routes
- **Composer les groupes** : Créer des sous-groupes avec des préfixes imbriqués

### Middlewares
Un middleware est une fonction qui s'exécute **avant** le handler principal. Elle peut :
- Valider la requête (authentification, autorisation)
- Modifier la réponse (ajouter des headers, CORS)
- Arrêter le traitement en retournant `false`

## Types de middlewares

```cpp
using Middleware = std::function<bool(const httplib::Request&, httplib::Response&)>;
```

Un middleware :
- Reçoit la requête et la réponse
- Retourne `true` pour continuer le traitement
- Retourne `false` pour arrêter et envoyer la réponse

## Utilisation basique

### 1. Middleware global

```cpp
dng::Server server("127.0.0.1", 6767);

// Ajouter un middleware qui s'applique à TOUTES les routes
server.use([](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    return true;
});
```

### 2. Route group simple

```cpp
// Créer un groupe /api
auto api = server.group("/api");

// Toutes les routes du groupe seront préfixées par /api
api.add_get_handler("/users", [](const httplib::Request&, httplib::Response& res) {
    // Route finale : GET /api/users
    res.set_content("Users list", "text/plain");
});
```

### 3. Middleware sur un groupe

```cpp
auto api = server.group("/api");

// Middleware d'authentification
api.use([](const httplib::Request& req, httplib::Response& res) {
    auto auth = req.get_header_value("Authorization");
    if (auth.empty()) {
        res.set_content("Unauthorized", "text/plain");
        res.status = 401;
        return false; // Arrête le traitement
    }
    return true; // Continue
});

// Cette route nécessite donc l'authentification
api.add_get_handler("/secret", [](const httplib::Request&, httplib::Response& res) {
    res.set_content("Secret data", "text/plain");
});
```

### 4. Sous-groupes (groupes imbriqués)

```cpp
// Groupe principal
auto api = server.group("/api");

// Sous-groupe /api/auth
auto auth = api.group("/auth");

auth.add_post_handler("/login", [](const httplib::Request&, httplib::Response& res) {
    // Route finale : POST /api/auth/login
    res.set_content("Login response", "text/plain");
});

// Un autre sous-groupe /api/users
auto users = api.group("/users");
users.use(auth_middleware); // Ce groupe nécessite l'authentification

users.add_get_handler("", [](const httplib::Request&, httplib::Response& res) {
    // Route finale : GET /api/users
    res.set_content("Users list", "text/plain");
});

users.add_post_handler("", [](const httplib::Request&, httplib::Response& res) {
    // Route finale : POST /api/users
    res.set_content("User created", "text/plain");
});
```

## Exemple complet : API RESTful

```cpp
#include "Server.hpp"

// Middleware d'authentification
bool auth_middleware(const httplib::Request& req, httplib::Response& res) {
    auto token = req.get_header_value("Authorization");
    if (token.empty() || token != "Bearer secret-token") {
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        res.status = 401;
        return false;
    }
    return true;
}

int main() {
    dng::Server server("127.0.0.1", 6767);

    // Routes publiques
    server.add_get_handler("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // Groupe /api
    auto api = server.group("/api");

    // Public : /api/auth
    auto auth = api.group("/auth");
    auth.add_post_handler("/login", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"token":"secret-token"})", "application/json");
        res.status = 200;
    });

    // Protégé : /api/account
    auto account = api.group("/account");
    account.use(auth_middleware);

    account.add_get_handler("", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"id":1,"name":"John"})", "application/json");
    });

    account.add_put_handler("", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"message":"Updated"})", "application/json");
    });

    // Protégé : /api/users
    auto users = api.group("/users");
    users.use(auth_middleware);

    users.add_get_handler("", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"([{"id":1,"name":"Alice"}])", "application/json");
    });

    users.add_post_handler("", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"id":2,"name":"Bob"})", "application/json");
        res.status = 201;
    });

    users.add_get_handler("/:id", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"id":1,"name":"Alice"})", "application/json");
    });

    users.add_put_handler("/:id", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"message":"Updated"})", "application/json");
    });

    users.add_delete_handler("/:id", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    server.start();
    return 0;
}
```

## Ordre d'exécution des middlewares

Quand une requête arrive :

1. **Middlewares globaux** (définis avec `server.use()`) s'exécutent en ordre
2. **Middlewares du groupe** (définis avec `group.use()`) s'exécutent en ordre
3. **Handler** s'exécute

Si n'importe quel middleware retourne `false`, le traitement s'arrête et la réponse est envoyée.

```
Request
  ↓
Global Middleware 1 (true)
  ↓
Global Middleware 2 (true)
  ↓
Group Middleware 1 (true)
  ↓
Handler
  ↓
Response
```

## Méthodes HTTP supportées

```cpp
group.add_get_handler("/path", handler);      // GET
group.add_post_handler("/path", handler);     // POST
group.add_put_handler("/path", handler);      // PUT
group.add_delete_handler("/path", handler);   // DELETE
group.add_patch_handler("/path", handler);    // PATCH
group.add_options_handler("/path", handler);  // OPTIONS
```
