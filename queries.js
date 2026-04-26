db = db.getSiblingDB('eventdb');

db.users.insertOne({
  login: "exg0rd",
  password_hash: "hash123",
  first_name: "Egor",
  last_name: "Sayapin",
  email: "egor.sayapin1337@gmail.com",
  created_at: new Date()
});

db.users.findOne({ login: "exg0rd" });

db.users.find({
  $and: [
    { first_name: { $regex: "Egor", $options: "i" } },
    { last_name: { $regex: "Sayapin", $options: "i" } }
  ]
});

db.events.insertOne({
  title: "New Event",
  description: "Event description",
  date: new Date("2026-12-25"),
  location: "Moscow",
  organizer_id: ObjectId("507f1f77bcf86cd799439011"),
  participants: [],
  created_at: new Date()
});

db.events.find();

db.events.find({
  date: {
    $gte: new Date("2026-06-01"),
    $lt: new Date("2026-07-01")
  }
});

db.events.updateOne(
  { _id: ObjectId("507f1f77bcf86cd799439011") },
  {
    $push: {
      participants: {
        user_id: ObjectId("507f191e810c19729de860ea"),
        registered_at: new Date()
      }
    }
  }
);

db.events.find({ "participants.user_id": ObjectId("507f191e810c19729de860ea") });

db.events.aggregate([
  { $unwind: "$participants" },
  { $match: { "participants.user_id": ObjectId("507f191e810c19729de860ea") } },
  { $project: { title: 1, date: 1, location: 1 } }
]);

db.events.updateOne(
  { _id: ObjectId("507f1f77bcf86cd799439011") },
  {
    $pull: {
      participants: { user_id: ObjectId("507f191e810c19729de860ea") }
    }
  }
);

db.events.updateOne(
  { _id: ObjectId("507f1f77bcf86cd799439011") },
  { $set: { title: "Updated Title", description: "Updated description" } }
);

db.users.deleteOne({ login: "exg0rd" });

db.events.deleteOne({ _id: ObjectId("507f1f77bcf86cd799439011") });

db.events.aggregate([
  { $match: { date: { $gte: new Date("2026-06-01") } } },
  {
    $project: {
      title: 1,
      date: 1,
      participant_count: { $size: "$participants" }
    }
  },
  { $sort: { participant_count: -1 } }
]);

db.events.aggregate([
  { $unwind: "$participants" },
  {
    $group: {
      _id: "$participants.user_id",
      event_count: { $sum: 1 }
    }
  },
  { $sort: { event_count: -1 } },
  { $limit: 10 }
]);
