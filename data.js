db = db.getSiblingDB('eventdb');

db.users.insertMany([
  {
    login: "jdoe",
    password_hash: "5ebe2294ecd0e0f08eab7690d2a6ee69",
    first_name: "John",
    last_name: "Doe",
    email: "jdoe@example.com",
    created_at: new Date("2026-01-15")
  },
  {
    login: "asmith",
    password_hash: "098f6bcd4621d373cade4e832627b4f6",
    first_name: "Alice",
    last_name: "Smith",
    email: "asmith@example.com",
    created_at: new Date("2026-01-20")
  },
  {
    login: "bwilliams",
    password_hash: "5f4dcc3b5aa765d61d8327deb882cf99",
    first_name: "Bob",
    last_name: "Williams",
    email: "bwilliams@example.com",
    created_at: new Date("2026-02-01")
  },
  {
    login: "cjohnson",
    password_hash: "e99a18c428cb38d5f260853678922e03",
    first_name: "Carol",
    last_name: "Johnson",
    email: "cjohnson@example.com",
    created_at: new Date("2026-02-10")
  },
  {
    login: "dbrown",
    password_hash: "fcea920f7412b5da7be0cf42b8c93759",
    first_name: "David",
    last_name: "Brown",
    email: "dbrown@example.com",
    created_at: new Date("2026-02-15")
  },
  {
    login: "ejones",
    password_hash: "d8578edf8458ce06fbc5bb76a58c5ca4",
    first_name: "Emma",
    last_name: "Jones",
    email: "ejones@example.com",
    created_at: new Date("2026-03-01")
  },
  {
    login: "fgarcia",
    password_hash: "25d55ad283aa400af464c76d713c07ad",
    first_name: "Frank",
    last_name: "Garcia",
    email: "fgarcia@example.com",
    created_at: new Date("2026-03-05")
  },
  {
    login: "gmartinez",
    password_hash: "e10adc3949ba59abbe56e057f20f883e",
    first_name: "Grace",
    last_name: "Martinez",
    email: "gmartinez@example.com",
    created_at: new Date("2026-03-10")
  },
  {
    login: "hrodriguez",
    password_hash: "827ccb0eea8a706c4c34a16891f84e7b",
    first_name: "Henry",
    last_name: "Rodriguez",
    email: "hrodriguez@example.com",
    created_at: new Date("2026-03-15")
  },
  {
    login: "iwilson",
    password_hash: "25f9e794323b453885f5181f1b624d0b",
    first_name: "Ivy",
    last_name: "Wilson",
    email: "iwilson@example.com",
    created_at: new Date("2026-03-20")
  }
]);

const users = db.users.find().toArray();

db.events.insertMany([
  {
    title: "Tech Conference 2026",
    description: "Annual technology conference",
    date: new Date("2026-06-15"),
    location: "Moscow",
    organizer_id: users[0]._id,
    participants: [
      { user_id: users[1]._id, registered_at: new Date("2026-04-01") },
      { user_id: users[2]._id, registered_at: new Date("2026-04-05") }
    ],
    created_at: new Date("2026-03-25")
  },
  {
    title: "Startup Meetup",
    description: "Networking event for startups",
    date: new Date("2026-05-20"),
    location: "Saint Petersburg",
    organizer_id: users[1]._id,
    participants: [
      { user_id: users[0]._id, registered_at: new Date("2026-04-10") },
      { user_id: users[3]._id, registered_at: new Date("2026-04-12") },
      { user_id: users[4]._id, registered_at: new Date("2026-04-15") }
    ],
    created_at: new Date("2026-04-01")
  },
  {
    title: "AI Workshop",
    description: "Hands-on machine learning workshop",
    date: new Date("2026-07-10"),
    location: "Kazan",
    organizer_id: users[2]._id,
    participants: [
      { user_id: users[5]._id, registered_at: new Date("2026-05-01") }
    ],
    created_at: new Date("2026-04-20")
  },
  {
    title: "DevOps Summit",
    description: "Cloud and DevOps best practices",
    date: new Date("2026-08-05"),
    location: "Novosibirsk",
    organizer_id: users[3]._id,
    participants: [],
    created_at: new Date("2026-05-01")
  },
  {
    title: "Web Development Bootcamp",
    description: "Intensive web dev training",
    date: new Date("2026-09-01"),
    location: "Yekaterinburg",
    organizer_id: users[4]._id,
    participants: [
      { user_id: users[6]._id, registered_at: new Date("2026-06-01") },
      { user_id: users[7]._id, registered_at: new Date("2026-06-05") }
    ],
    created_at: new Date("2026-05-15")
  },
  {
    title: "Blockchain Forum",
    description: "Cryptocurrency and blockchain discussion",
    date: new Date("2026-10-12"),
    location: "Moscow",
    organizer_id: users[5]._id,
    participants: [
      { user_id: users[8]._id, registered_at: new Date("2026-07-01") }
    ],
    created_at: new Date("2026-06-01")
  },
  {
    title: "Mobile App Hackathon",
    description: "24-hour mobile development competition",
    date: new Date("2026-11-20"),
    location: "Saint Petersburg",
    organizer_id: users[6]._id,
    participants: [
      { user_id: users[0]._id, registered_at: new Date("2026-08-01") },
      { user_id: users[9]._id, registered_at: new Date("2026-08-10") }
    ],
    created_at: new Date("2026-07-01")
  },
  {
    title: "Cybersecurity Conference",
    description: "Information security trends",
    date: new Date("2026-12-05"),
    location: "Kazan",
    organizer_id: users[7]._id,
    participants: [],
    created_at: new Date("2026-08-01")
  },
  {
    title: "Data Science Meetup",
    description: "Big data and analytics",
    date: new Date("2027-01-15"),
    location: "Novosibirsk",
    organizer_id: users[8]._id,
    participants: [
      { user_id: users[1]._id, registered_at: new Date("2026-09-01") },
      { user_id: users[2]._id, registered_at: new Date("2026-09-05") },
      { user_id: users[3]._id, registered_at: new Date("2026-09-10") }
    ],
    created_at: new Date("2026-08-15")
  },
  {
    title: "Game Development Workshop",
    description: "Unity and Unreal Engine basics",
    date: new Date("2027-02-20"),
    location: "Yekaterinburg",
    organizer_id: users[9]._id,
    participants: [
      { user_id: users[4]._id, registered_at: new Date("2026-10-01") }
    ],
    created_at: new Date("2026-09-01")
  }
]);

db.users.createIndex({ "login": 1 }, { unique: true });
db.users.createIndex({ "first_name": 1, "last_name": 1 });
db.events.createIndex({ "date": 1 });
db.events.createIndex({ "organizer_id": 1 });
db.events.createIndex({ "participants.user_id": 1 });

print("Data inserted successfully");
