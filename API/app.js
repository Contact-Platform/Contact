/*eslint-env node*/
const bodyParser = require('body-parser');
const date = require('date-and-time');
const express = require('express');
const cfenv = require('cfenv');
const cors = require('cors');
const app = express();
const appEnv = cfenv.getAppEnv();

/** 
 * *****************************************************************
 * 
 * START WATSON Service Credentials
 * replace the credentials with with your own credentials.
 * 
 * *****************************************************************
 */

// Natural Language Understanding Authentication Credentials
const NLUV1 = require('watson-developer-cloud/natural-language-understanding/v1');
const NLU = new NLUV1({
  version: 'YOUR_VERSION_HERE',
  username: 'YOUR_USERNAME_HERE',
  password: 'YOUR_PASSWORD_HERE'
});

// Tone Analyzer Authentication Credentials
const TAV3 = require('watson-developer-cloud/tone-analyzer/v3');
const TA = new TAV3({
  version: 'YOUR_VERSION_HERE',
  username: 'YOUR_USERNAME_HERE',
  password: 'YOUR_PASSWORD_HERE'
});

// Cloudant Database Authentication Credentials
const Cloudant = require("@cloudant/cloudant");
const cloudant = Cloudant({
  account: 'YOUR_USERNAME_HERE',
  password: 'YOUR_PASSWORD_HERE'
});

// Cloudant databases
const NLU_DB = 'YOUR_NLU_DB_NAME_HERE';
const TA_DB = 'YOUR_TA_DB_NAME_HERE';
const location_DB = 'YOUR_LOCATION_DB_NAME_HERE';

/**
 * ***************************************************************** 
 * 
 * END WATSON Service Credentials
 * 
 * *****************************************************************
 */

app.use(cors());

app.use(bodyParser.urlencoded({
  extended: false
}));

app.use(bodyParser.json());

app.use(express.static(__dirname + '/public'));

app.listen(appEnv.port, '0.0.0.0', function () {
  console.log("server starting on " + appEnv.url);
});

/** This function returns a date object i.e.
 * 
 *  date = {
 *    month: {
 *      number: 1 - 12,
 *      name: 'January' - 'December'
 *    },
 *    day: {
 *      number: 1 - 31,
 *      name: 'Sunday' - 'Saturday'
 *    },
 *    year: 0999 - today
 *  }
 */
function getDate() {
  let dateObj = {};
  let today = date.format(new Date(), 'M,MMMM,D,dddd,YYYY').split(',');

  dateObj['month'] = {
    number: Number(today[0]),
    name: today[1]
  };

  dateObj['day'] = {
    number: Number(today[2]),
    name: today[3]
  };

  dateObj['year'] = Number(today[4]);

  return dateObj;
}

/** Watson Analysis and Store in database.
 * Post Request expected structure
 * 
 * request = {
 *  content: text
 * }
 */
app.post('/api/analyze', analyzeText);

function analyzeText(req, res) {
  // console.log('Analyze POST request made...');
  let text = req.body.content.toLowerCase();

  // Natural Language Understanding API parameters
  let nluParams = {
    'language': "en",
    'text': text,
    'features': {
      'sentiment': {},
      'emotion': {},
      'keywords': {},
      'entities': {}
    }
  };

  NLU.analyze(nluParams, (err, res) => {
    if (err) {
      console.log(err);
    }
    res.date = getDate();
    let db = cloudant.db.use(NLU_DB);
    db.insert(res, (err, data) => {
      if (err)
        console.log('NLU error: ', err);
      else
        console.log('NLU success: ', data)
    });
  });

  // Tone Analyzer API parameters
  let taParams = {
    'tone_input': {
      'text': text
    },
    'content_type': 'application/json'
  };

  TA.tone(taParams, (err, res) => {
    if (err) {
      console.log(err);
    }

    if (res.document_tone.tones.length != 0) {
      res.date = getDate();
      let db = cloudant.db.use(TA_DB);
      db.insert(res, (err, data) => {
        if (err)
          console.log('TA error: ', err);
        else
          console.log('TA success: ', data)
      });
    }
  });

}

/** Add Device Location to Database
 * Post Request expected structure
 * 
 * request = {
 *  id: id
 *  lat: number,
 *  lon: number,
 *  description: string,
 * }
 */
app.post('/api/add/location', addLocation);

function addLocation(req, res) {
  let db = cloudant.db.use(location_DB);
  let doc = {
    deviceId: req.body.id, // device id
    lat: req.body.lat, // device latitud
    lon: req.body.lon, // device longitud
    description: req.body.description, //device description
    time: date.format(new Date(), 'HH:mm')
  }

  db.insert(doc, (err, data) => {
    if (err) {
      console.log(err);
    } else {
      console.log(data);
    }
  })
}

app.get('/api/get/all/:database_name', getAllDb);

function getAllDb(req, res) {
  let database_name = req.params['database_name'];
  let db = cloudant.db.use(database_name);
  let params = {
    include_docs: true
  };


  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }
    res.send(data);
  });
}

// HTTP GET Request for locations
app.get('/api/get/geoJSON', getGeoJSON);

function getGeoJSON(req, res) {
  let db = cloudant.db.use(location_DB);
  let params = {
    include_docs: true
  };

  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }

    let geoJSON = {
      type: 'Feature Collection',
      features: []
    };

    for (let row of data.rows) {
      let feature = {
        type: 'Feature',
        geometry: {
          type: 'Point',
          coordinates: [row.doc.lon, row.doc.lat]
        },
        properties: {
          title: row.doc.deviceId,
          description: row.doc.description
        }
      };

      geoJSON.features.push(feature)
    }

    res.send(geoJSON);
  });
}

// HTTP GET Request for wordcloud
app.get('/api/get/wordcloud', getWordcloud);

function getWordcloud(req, res) {
  let db = cloudant.db.use(NLU_DB);
  let params = {
    include_docs: true
  };


  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }

    var wordCount = {};
    for (let row of data.rows) {
      for (let keyword of row.doc.keywords) {
        let word = keyword.text.toLowerCase();
        if (wordCount[word]) {
          wordCount[word]++;
        } else {
          wordCount[word] = 1;
        }
      }
    }

    res.send(wordCount);
  });
}

// HTTP GET Request for Sentiment Analysis
app.get('/api/get/sentiment', getSentiment);

function getSentiment(req, res) {
  let db = cloudant.db.use(NLU_DB);
  let params = {
    include_docs: true
  };


  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }

    let msgObj = {
      total: data.total_rows
    };
    for (let row of data.rows) {
      let prop = row.doc.sentiment.document.label;
      if (msgObj[prop]) {
        msgObj[prop]++;
      } else {
        msgObj[prop] = 1;
      }
    }

    res.send(msgObj);
  });
}

// HTTP GET Request for sentiment Chart.js datasets 
app.get('/api/get/sentiment/datasets', getSentimentDatasets);

function getSentimentDatasets(req, res) {
  let db = cloudant.db.use(NLU_DB);
  let params = {
    include_docs: true
  };


  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }

    let obj = {};
    let dateArr = [];
    let labels = [];

    for (let row of data.rows) {
      let date = row.doc.date;
      if (dateArr.indexOf(date.day.number) == -1) {
        dateArr.push(date.day.number)
        labels.push(date.day.number + '-' + date.month.number + '-' + date.year);
      }
    }

    dateArr.sort();
    obj.labels = labels.sort();
    obj.datasets = [{
      label: "Positive",
      backgroundColor: '#38C172',
      borderColor: '#38C172',
      fill: false,
      data: []
    }, {
      label: "Negative",
      backgroundColor: '#E3342F',
      borderColor: '#E3342F',
      fill: false,
      data: []
    }, {
      label: "Neutral",
      backgroundColor: '#F6993F',
      borderColor: '#F6993F',
      fill: false,
      data: []
    }];

    for (let dataset of obj.datasets) {
      dataset.data = new Array(dateArr.length).fill(0);
    }

    for (let row of data.rows) {
      let label = row.doc.sentiment.document.label;
      switch (label) {
        case 'positive':
          updateDataset(dateArr, obj.datasets[0], row.doc)
          break;
        case 'negative':
          updateDataset(dateArr, obj.datasets[1], row.doc)
          break;
        case 'neutral':
          updateDataset(dateArr, obj.datasets[2], row.doc)
          break;
      }
    }

    res.send(obj);
  });

  function updateDataset(days, obj, data) {
    let index = days.indexOf(data.date.day.number)
    obj.data[index]++;
  }
}

// HTTP GET Request for tone Chart.js datasets 
app.get('/api/get/tone/datasets', getToneDatasets);

function getToneDatasets(req, res) {
  let db = cloudant.db.use(TA_DB);
  let params = {
    include_docs: true
  };

  db.list(params, function (err, data) {
    if (err) {
      console.log(err);
      return;
    }

    let chartData = {
      labels: [],
      datasets: [{
        label: "tone amount",
        backgroundColor: '#3490DC',
        borderColor: '#3490DC',
        data: []
      }]
    };

    for (let row of data.rows) {
      for (let tone of row.doc.document_tone.tones) {
        let name = tone.tone_name;
        if (chartData.labels.indexOf(name) == -1)
          chartData.labels.push(name)
      }
    }

    chartData.labels.sort()
    chartData.datasets[0].data = new Array(chartData.labels.length).fill(0);

    for (let row of data.rows) {
      for (let tone of row.doc.document_tone.tones) {
        let index = chartData.labels.indexOf(tone.tone_name);
        chartData.datasets[0].data[index]++;
      }
    }

    res.send(chartData);
  });
}