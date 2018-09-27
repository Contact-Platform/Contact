<div id="top"></div>

# Contact's Node.js API

[contact-home]: https://contact-app.mybluemix.net/
In this project we demonstrate our Node.js API, based on the Express framework, who is **designed to help _you_ understand what your users are saying.** The API receives messages and analyzes them for sentiment and emotional tone. We store all the analysis data in a database and visualize it through charts, graphs and maps to help you make appropriate and informed decisions. [Click here][contact-home] to see it up and running.  

**Note:** This is meant to be an **Add-On** to any existing application, in our case we use [Contact][contact-home]. Once you do the steps you're expected to develop your own external application to feed this API. Keep that in mind, you're supposed to **bring your own data**. That said we do give you a very basic way of data entry.

**Side Note:** If you think you can help us visualize _our_ data in other cool and creative ways you can visit [this jsfiddle](https://jsfiddle.net/janielMartell/cmf7n031/10/).  

**Table of Contents**  
[Overview](##Overview "Jump to Overview")  
[Steps](##Steps "Jump to Steps")  
[Sample Output](##Sample-Output "Jump to Sample Output")  
[How to use it](##How-to-use-it)  
[Deploying your API](##Deploy-your-API-to-the-IBM-Cloud "Jump to Deploy your API to the IBM Cloud")  
[Demo](##Demo "Jump to Demo")  
[Technologies](##Technologies "Jump to Technologies")  
[Components](##Components "Jump to Components")  

## Overview

[architecture]: ./images/architecture.png  "Project Flow Diagram"
![alt text][architecture]

1. Send messages through [**Contact**][contact-home], in our case, or through _anything_ connected to the internet, your case.
1. Run Node.js application that receives those messages _via_ POST requests.
1. Send them to Watson Natural Language Understanding (**NLU**) for processing.
1. Send them to Watson Tone Analyzer (**TA**) for analysis.
1. Store the results in a Cloudant Database.
1. Present them trough an user interface, in this case, a web app with charts, graphs and maps.  
1. Lastly, sit back and enjoy Watson's, and our, hard work. 😉

## Steps

**Quick links**  
[Step 1. Clone the repository](###-1.-Clone-the-repository)  
[Step 2. cd into this project's root directory](###-2.-cd-into-this-project's-root-directory)  
[Step 3. Make sure Node.js is installed in your computer](###-3.-Make-sure-is-installed-in-your-computer)  
[Step 4. Install dependencies](###-4.-Install-dependencies)  
[Step 5. Create Watson services with IBM Cloud](###-5.-Create-Watson-services-with-IBM-Cloud)  
[Step 6. Configure your Credentials](###-6.-Configure-your-Credentials)  
[Step 7. Run your API locally](###-7.-Run-your-API-locally)

### 1. Clone the repository

```bash
git clone https://github.com/Contact-Platform/Contact.git
```

### 2. cd into this project's root directory

```bash
cd Contact
```

### 3. Make sure [Node.js](https://nodejs.org/en/download/ "Install Node.js") is installed in your computer

```bash
node -v
# v10.1.0 expected output
npm -v
# 6.4.1 expected output
```

### 4. Install dependencies

```bash
npm install
```

### 5. Create Watson services with IBM Cloud

> **Pro Tip:** Do this step on Chrome or Firefox because the IBM Cloud is **awfully slow** on other browsers (e.g. Opera).  
> \- [Friendly Neighborhood Developer](https://github.com/janielMartell/)

If you don't have an IBM Cloud account, [sign up](https://console.bluemix.net/registration/ "Bluemix sign up page") for Bluemix and,  
Create the following services:

- [Watson Natural Language Understanding (NLU)](####Create-Watson-NLU-Service "Jump to NLU")
- [Watson Tone Analyzer (TA)](####Create-Watson-TA-Service "Jump to TA")
- [IBM Cloudant Database](####Create-IBM-Cloudant-Database-Service "Jump to Cloudant")

[IBM Cloud Catalog]: https://console.bluemix.net/catalog/

#### Create Watson NLU Service

1. [Click here](https://console.bluemix.net/catalog/services/natural-language-understanding "Create NLU Service") to create the service or go to the [IBM Cloud Catalog] and search for 'label:lite natural language understanding'.
1. Name your service.
1. Choose your pricing plan. (lite)
1. Click Create.
1. Click Service credentials.
1. Find the Auto-generated service credentials and click _view credentials_.
1. Copy credentials into a text file or keep the tab open because you'll need it in [step 6](###6.-Configure-Your-Credentials).

#### Create Watson TA Service

1. [Click here](https://console.bluemix.net/catalog/services/tone-analyzer "Create TA Service") to create the service or go to the [IBM Cloud Catalog] and search for 'label:lite tone analyzer'.
1. Name your service.
1. Choose your pricing plan. (lite)
1. Click Create.
1. Click Show Credentials.
1. Copy credentials into a text file or keep the tab open because you'll need it in [step 6](###6.-Configure-Your-Credentials).

#### Create IBM Cloudant Database Service

1. [Click here](https://console.bluemix.net/catalog/services/cloudant "Create Cloudant Service") to create the service or go to the [IBM Cloud Catalog] and search for 'label:lite cloudant'.
1. Name your service.
1. Choose an authentication method. (Use both)
1. Choose your pricing plan. (lite)
1. Click Create.
1. Click Service credentials.
1. Click New Credential and Add.
1. Click _view credentials_.
1. Copy credentials into a text file or keep the tab open because you'll need it in [step 6](###6.-Configure-Your-Credentials).
1. Go back to the Manage tab.
1. Click Launch Cloudant Dashboard button.  

    > **Pro Tip:** When using Chrome on my Desktop the 'Launch Cloudant Dashboard' button mentioned in step 11 didn't show up, if that's the case go to: `[YOUR_CLOUDANT_USERNAME].cloudant.com/dashboard.html#/_all_dbs`.  
    > \- [Friendly Neighborhood Developer](https://github.com/janielMartell/)

1. Create 3 databases and **name** them, keep in mind that:  
    1. One is for your Natural Language Understanding results,
    1. The other one is for your Tone Analyzer results, and,
    1. The last one is for some locations (that will show up on a map).
1. Save those names into a text file or keep the tab open because you'll need it in [step 6](###6.-Configure-Your-Credentials).

### 6. Configure your Credentials

Now, I hope you did that text file thing I told you about or at least kept those tabs open because the time has come, you're going to need it. Find the following, in `app.js`, and replace the credentials with your own. You can find the version for [NLU](https://www.ibm.com/watson/developercloud/natural-language-understanding/api/v1/?node#versioning) and [TA](https://www.ibm.com/watson/developercloud/tone-analyzer/api/v3/node.html?node#versioning) in their respective API reference.

#### app.js

```javascript
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
  version:  'YOUR_VERSION_HERE',
  username: 'YOUR_USERNAME_HERE',
  password: 'YOUR_PASSWORD_HERE'
});

// Tone Analyzer Authentication Credentials
const TAV3 = require('watson-developer-cloud/tone-analyzer/v3');
const TA = new TAV3({
  version:  'YOUR_VERSION_HERE',
  username: 'YOUR_USERNAME_HERE',
  password: 'YOUR_PASSWORD_HERE'
});

// Cloudant Database Authentication Credentials
const Cloudant = require("@cloudant/cloudant");
const cloudant = Cloudant({
  account:  'YOUR_USERNAME_HERE',
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
```

### 7. Run your API locally

 1. To run your API locally run `npm start`.
 1. You can access the running app in a browser at <http://localhost:6002>.
 1. Start typing things into the form and either wait for it to automatically refresh or refresh the page to see the what watson found on your messages.

 > **Pro Tip:** If you plan to make some changes, which I hope you do, and you don't want to shut your server on and off a million times; run `npm i -g nodemon` and `nodemon`. This will automatically restart your server whenever you save.  
> \- [Friendly Neighborhood Developer](https://github.com/janielMartell/)

## Sample Output

[sample00]: ./images/00SampleOutput.png 
![alt text][sample00]

[sample01]: ./images/01SampleOutput.png 
![alt text][sample01]

[sample02]: ./images/02SampleOutput.png 
![alt text][sample02]

[sample03]: ./images/03SampleOutput.png 
![alt text][sample03]

## How to use it

Now that you have everything up and running I bet you're wondering how to use this thing and it's very simple (kinda). Let's start with how to input data.

### Data input

In our case we use [Contact][contact-home] as our external data input for the API. In this repository we are going to use forms as examples on how to do it. As we mentioned in the beginning this is meant to be an **Add-On** to your applications so we think these examples are enough to help you understand the concepts for you to apply them and make this your own.

We have **two** forms `analyze.html` and `location.html`.

#### analyze.html

You can access this through <http://localhost:6002/analyze.html>. To input data to the API we start by:

1. Taking the content you want analyzed and converting it into a JSON String,
    ``` javascript
    let obj = {
        content: "" // your input here!
    }

    // make it a json string
    let jsonString = JSON.stringify(obj);
    ```

1. Creating a new HTTP POST Request to the [/api/analyze](####/api/analyze) route, and,
    ``` javascript
    let request = new XMLHttpRequest(); //new HTTP Request
    request.open('POST', './api/analyze', true); //route
    request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
    ```

1. Sending it.  
    ``` javascript
    request.send(jsonString); // send the object to the API
    ```
Here's an example of it all together:  

``` html
<form id="form">
    <input required id="input" type="text">
    <button type="submit">Submit</button>
</form>
<script>
    document.getElementById('form').addEventListener('submit', () => {
        event.preventDefault(); //Don't reload page

        // content you want analyzed
        let input = document.getElementById('input').value;

        // make it a json string
        let data = JSON.stringify({
            content: input // make sure you use the content property
        })

        var request = new XMLHttpRequest(); //new HTTP Request
        request.open('POST', './api/analyze', true); //route
        request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

        request.send(data); // send the json string to the API
    });
</script>
```

#### location.html

You can access this through <http://localhost:6002/location.html>. What we do here is basically the same thing, we take all the different inputs make them a JSON string and send them to our [/api/add/location](####/api/add/location) route:

``` html
<form id="form">
    <input id="id">
    <input id="lat">
    <input id="lon">
    <textarea id="description"></textarea>
    <button type="submit">Submit</button>
</form>
<script>
    document.getElementById('form').addEventListener('submit', () => {
    event.preventDefault(); //Don't reload page

    // make inputs a json string
    let data = JSON.stringify({
        id: document.getElementById('id').value,
        lat: document.getElementById('lat').value,
        lon: document.getElementById('lon').value,
        description: document.getElementById('description').value
    })

    var request = new XMLHttpRequest(); //new HTTP Request
    request.open('POST', './api/add/location', true); // route
    request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

    request.send(data); // send the json string to the API
    });
</script>
```

### Routes

We've mentioned ___routes___ a couple of times so it's about time we explain what we're talking about. Routes are the way our API interacts with its various Components, through them we receive and send data. There are **three main routes** to this API, **two** for data input and **one** for data output.

#### Input routes

##### ___/api/analyze___

This is the star of the show, **why?** because when you send data to this route the API sends it to Watson NLU and TA to be processed, analyzed and stored in your databases. It does this by:

1. Receiving the input and creating the parameters for the Watson APIs.  
    ``` javascript
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

    // Tone Analyzer API parameters
    let taParams = {
        'tone_input': {
        'text': text
        },
        'content_type': 'application/json'
    };
    ```
1. Sending the data off to the APIs.
    ``` javascript
    NLU.analyze(nluParams, nluCallback);

    TA.tone(taParams, taCallback);
    ```
1. Saving their Response in your database
    ``` javascript
    function nluCallback(err, res) {
        let db = cloudant.db.use(NLU_DB);
        db.insert(res, (err, data) => {
            if (err)
                console.log('NLU error: ', err);
            else
                console.log('NLU success: ', data)
        });
    }

    function taCallback(err, res) {
        let db = cloudant.db.use(TA_DB);
        db.insert(res, (err, data) => {
            if (err)
            console.log('TA error: ', err);
            else
            console.log('TA success: ', data)
        });
    }
    ```

**Note:** We are using this to analyze text but if you go to
`app.js` you can change the parameters so that you can send text, HTML or even URLs to public webpages, like news articles, to be analyzed. Check out the API Reference for [NLU](https://www.ibm.com/watson/developercloud/natural-language-understanding/api/v1/?node#versioning) and [TA](https://www.ibm.com/watson/developercloud/tone-analyzer/api/v3/node.html?node#versioning) to learn more.

##### ___/api/add/location___

This route does pretty much the same thing but it's _way_ simpler. Here we don't use watson or anything we just send locations to be added to a your locations database. Like this:  

``` javascript
let doc = {
    deviceId: req.body.id,
    lat: req.body.lat,
    lon: req.body.lon,
    description: req.body.description,
    time: date.format(new Date(), 'HH:mm')
}

let db = cloudant.db.use(location_DB);
db.insert(doc, (err, data) => {
    if (err) {
        console.log(err);
    } else {
        console.log(data);
    }
})
```

#### Output routes

##### ___/api/get/all/:database_name___

This is the main output route of the API, **why?** well because this route returns every single row in your database you just have to pass the name of the database as a parameter.  

If you followed all the steps you should have three databases so go to <http://localhost:6002/api/get/all/> and add the name of one of your databases, you should see a response full of unformatted JSON. Now, you get to decide what you do with this. It's obviously not very pretty, readable or useable but with some time and effort you can try and make [something beautiful](##Demo) and useful out of them.

##### ___/api/get/___

We have more output routes but these output routes basically do the same thing as the main one, they're not necessary but they make our life easier. Instead of returning everything inside our database as is, it returns our data structured so that they're easier to work with. For example:

- [Chart.js](https://www.chartjs.org/): Simple yet flexible JavaScript charting for designers & developers.  

  We use our `/api/get/sentiment/datasets` and `/api/get/tone/datasets` routes to get our tone and sentiment data structured and ready to be visualized with Chart.js
  
- [D3](https://d3js.org/): JavaScript library for manipulating documents based on data.  

  We use our `/api/get/wordcloud` route to get our keywords data structured and ready to be visualized with a [Wordcloud](https://www.jasondavies.com/wordcloud/)

- [Leaflet](https://leafletjs.com/): Open-source JavaScript library for mobile-friendly interactive maps.  

  We use our `/api/get/geoJSON` route to get our location database structured in GeoJSON for it to be displayed in a nice little map.  

These are just three of examples of tools we used to visualize our data but you can use pretty much anything and we highly encourage you to try new things and create your own routes to be used with any number of tools available to you.

## Deploy your API to the IBM Cloud

Once you've played around with this and you're pleased with what you have, you can deploy your API to the IBM Cloud. This will allow you and others to interact with it through the internet and it'll give it a nice URL.

1. [Click here]( https://console.bluemix.net/catalog/starters/sdk-for-nodejs "Create Cloud Foundry App") to create the service or go to the [IBM Cloud Catalog] and search for 'SDK for Node.js'.
1. Name your application, FYI: The name will be displayed publicly through the URL.
1. Choose your pricing plan. (lite and 256 MB).
1. Click Create.
1. Got to the Overview tab and scroll down to Continuous Delivery.
1. Click Enable to enable Continuous Delivery, this will take you to a new page.
1. Scroll down to Tool Integrations and in repository type select new.
1. Now go to Delivery Pipeline.
1. It'll ask for an API Key, click the create button, to its right, and confirm it.  

    > **Side note:** Someone should change the this button's text to 'Generate' or something because having two buttons that pretty much look the same and that say the same thing is pretty confusing for the reader.

1. Now, the input should now be filled in, click Create (the one at the bottom).
1. Wait for everything to be configured and click Git to see your repo.
1. At the top it might let you know something about not being able to pull or push project code until you create a personal access token, if so:
    1. Click 'create a personal access token'.
    1. Here write the same name you gave your app, for convenience.
    1. Select api.
    1. Click Create personal access token.
    1. Now, copy that personal access token into a text file and **save it very, very well** because you won't be able to access it again and the command line **will** prompt you for it.
1. Copy the web URL. It should look something like this:  

    [git-repo-example]:  ./images/git-repo-example.png  "Project Flow Diagram"
    ![alt text][git-repo-example]

1. From your project's root run:
    ``` bash
    git remote rename origin old-origin
    git remote add origin [YOUR_WEB_URL]
    git push -u origin master
    ```

And that should be it you can access your application by going to your App's Overview tab and clicking Visit App URL.

## Demo

As mentioned at the top we have an example of this up and running [Click here][contact-home] to see it.

### Images

Here are some screenshots of what we did with ours so feel free to make this your own.  

[hero]:  ./images/hero.png
![alt text][hero]

[wordcloud]:  ./images/wordcloud.png
![alt text][wordcloud]

[graphs]:  ./images/graphs.png
![alt text][graphs]


## Technologies

- [Contact][contact-home]: A Real Time Open Communication Platform.
- [Node.js®](https://nodejs.org/ "Node.js Homepage"): An open-source JavaScript runtime built on [Chrome's V8 JavaScript engine](https://developers.google.com/v8/ "V8's wiki") designed to build scalable network applications.
- [Express](https://expressjs.com/ "Express Homepage"): Fast, unopinionated, minimalist web framework for Node.js
- [Artificial Intelligence (Watson)](https://www.ibm.com/watson/about/index.html "IBM's Watson Homepage"): Powered by the latest innovations in machine learning, Watson lets you learn more with less data. You can integrate AI into your most important business processes, informed by IBM’s rich industry expertise.
- [NoSQL Database](https://en.wikipedia.org/wiki/NoSQL "NoSQL Wikipedia Article"): It provides a mechanism for storage and retrieval of data that is modeled in means other than the tabular relations used in relational databases.

## Components

- [Watson Natural Language Understanding (NLU)](https://www.ibm.com/watson/services/natural-language-understanding/ "NLU Homepage"): Natural language processing for advanced text analysis.
- [Watson Tone Analyzer (TA)](https://www.ibm.com/watson/services/tone-analyzer/ "TA Homepage"): Understand emotions and communication style in text.
- [IBM Cloudant](https://www.ibm.com/cloud/cloudant "Cloudant Homepage"): A scalable JSON document database for web, mobile, IoT and serverless applications.  
- [Cloud Foundry](https://www.cloudfoundry.org/ "Cloud Foundry Homepage"): Makes it faster and easier to build, test, deploy and scale applications, providing a choice of clouds, developer frameworks, and application services. It's open source and available through a variety of private cloud distributions and public cloud instances (e.g. IBM Cloud).
- [IBM Continous Delivery](https://www.ibm.com/cloud/continuous-delivery): Embrace DevOps in an enterprise-ready way. Create toolchains that support your app delivery tasks. Automate builds, tests, deployments, and more.  

[Jump back to top](#top)