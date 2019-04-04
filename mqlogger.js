let mqtt = require('mqtt')
let client  = mqtt.connect('##MQ-Broker-Address##')
let MongoClient = require('mongodb').MongoClient
let url = "##mongodb-URL##"
var express = require('express');
var app = express();

/* Use if mongo server is on the same device as logger
app.use(function(req, res, next) {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  next();
});
*/


app.get('/mqtt', function(req, res){
  var topic = req.query.topic
  var message = req.query.message
  res.send('Get: ' + topic + ' ' + message)
  console.log('Get: ' + topic + ' ' + message)
  client.publish(topic, message)

  MongoClient.connect(url, { useNewUrlParser: true }, function(err, db) {
    if (err) throw err
    var dbo = db.db("##Your-Database##")
    var mytemp = dbo.collection("##YourCollection##").aggregate([
      { $match: { $and: [ { tags: { $nin: [ "update"] } }, { tags: { $in: [ "temp"] } }] } },
      { $group: { _id: "$topic" } }
    ]).toArray(
      function(err, results){
        console.log(results)
      }
    )
        db.close()
      })


})
app.listen(4000)



client.on('connect', function () {
  client.subscribe('/home/#', function (err) {
    console.log("Connecting to MQTT...")
    if (!err) {
      console.log("connected")
    } else {
      console.log("Unable to connect")
    }
  })
})

/*

Logger will split topics into an array tags ex:

/home/temp/upstairs/laundry

"tags":["home","temp","upstairs","laundry"]

Will also log temp readings to "temp":

*/

client.on('message', function (topic, message) {

  MongoClient.connect(url, { useNewUrlParser: true }, function(err, db) {
    if (err) throw err
    var dbo = db.db("##Your-Database##")
    if (topic.split('/').includes('temp')) {
      var myobj = { topic: topic, tags: topic.split('/').slice(1), temp: Number(message.toString()), added: new Date() };
      dbo.collection("##YourCollection##").insertOne(myobj, function(err, res) {
        if (err) throw err
        db.close()

      })
      console.log(myobj.topic + ":" + myobj.temp)
    }else {
      var myobj = { topic: topic, tags: topic.split('/').slice(1), action: message.toString(), added: new Date() };
      dbo.collection("##YourCollection##").insertOne(myobj, function(err, res) {
        if (err) throw err
        db.close()

      })

      /* for debugging */
      console.log(myobj.topic + ":" + myobj.action)

    }
  })
})
