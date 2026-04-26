db = db.getSiblingDB('eventdb');

db.createCollection("users", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["login", "password_hash", "first_name", "last_name", "email"],
      properties: {
        login: {
          bsonType: "string",
          pattern: "^[a-zA-Z0-9_]{3,20}$",
          description: "must be a string 3-20 chars"
        },
        password_hash: {
          bsonType: "string",
          minLength: 8,
          description: "must be a string at least 8 chars"
        },
        first_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 50,
          description: "must be a string 1-50 chars"
        },
        last_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 50,
          description: "must be a string 1-50 chars"
        },
        email: {
          bsonType: "string",
          pattern: "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$",
          description: "must be a valid email"
        },
        created_at: {
          bsonType: "date",
          description: "must be a date"
        }
      }
    }
  }
});

db.createCollection("events", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["title", "date", "organizer_id"],
      properties: {
        title: {
          bsonType: "string",
          minLength: 3,
          maxLength: 200,
          description: "must be a string 3-200 chars"
        },
        description: {
          bsonType: "string",
          maxLength: 2000,
          description: "must be a string max 2000 chars"
        },
        date: {
          bsonType: "date",
          description: "must be a date"
        },
        location: {
          bsonType: "string",
          maxLength: 200,
          description: "must be a string max 200 chars"
        },
        organizer_id: {
          bsonType: "objectId",
          description: "must be an objectId"
        },
        participants: {
          bsonType: "array",
          items: {
            bsonType: "object",
            required: ["user_id", "registered_at"],
            properties: {
              user_id: {
                bsonType: "objectId"
              },
              registered_at: {
                bsonType: "date"
              }
            }
          }
        },
        created_at: {
          bsonType: "date",
          description: "must be a date"
        }
      }
    }
  }
});

db.users.insertOne({
  login: "ab",
  password_hash: "short",
  first_name: "Test",
  last_name: "User",
  email: "invalid-email"
});

db.events.insertOne({
  title: "AB",
  date: "not-a-date",
  organizer_id: "not-an-objectid"
});

print("Validation schema created");
