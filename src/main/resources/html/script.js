/**
 * Script for the visualizer webpage.
 * Author: Sibert Aerts
 */

$(document).ready(function(){
    // Register the file-loading event
    visualizer = new Visualizer('.file-input', '.day-control', '#view');

    $(".panel").draggable({
      handle: ".panel-head"
  });
})

//--------------------//
// File reading stuff //
//--------------------//

/// Handler passed to the file selector, called when a file is selected.
function readSingleFile(f, handler) {
    var file = f.target.files[0];
    if (!file) {
        console.log("File not found!");
        return;
    }
    var reader = new FileReader();
    reader.onload = handler;
    reader.readAsText(file);
}

/// Cleans up the JSON that boost generated by parsing strings back to numerics where needed.
function cleanData(data){
    // replace the dict with an array
    var townArray = [];
    for(var i = 0; i < Object.keys(data.towns).length; i++){
        var town = data.towns[""+ i];
        town.size = parseInt(town.size);
        town.lat = parseFloat(town.lat);
        town.long = parseFloat(town.long);
        townArray.push(town);
    }
    console.log(townArray.length);
    data.towns = townArray;

    for(var i in data.days){
        day = data.days[i];
        for(var id in day)
            day[id] = parseInt(day[id]);
    }
}

//------------//
// Class: Map //
//------------//

/// Make a new Map by the following attributes
/// imageRef: ref to the map image
/// ratio: horizontal/vertical ratio of the image
/// min/maxLat/Long: bounding box of the map image
var Map = function(name, imageRef, imageRatio, minLat, maxLat, minLong, maxLong, zoomable=false){
    this.name = name;
    this.imageRef = imageRef;
    this.imageRatio = imageRatio;
    this.zoomable = zoomable;
    this.box = {minLat: minLat, maxLat: maxLat, minLong: minLong, maxLong: maxLong};
}

/// Test if the given box fits inside the map.
Map.prototype.containsBox = function(box){
    if(this.box.minLat > box.minLat)
        return false;
    if(this.box.maxLat < box.maxLat)
        return false;
    if(this.box.minLong > box.minLong)
        return false;
    if(this.box.maxLong < box.maxLong)
        return false;
    return true;
}

// Returns a {width, height} object that matches the image's ratio that fits inside the given width and height
Map.prototype.fitTo = function(width, height){
    if(this.imageRatio > width/height)
        return {width: width, height: width / this.imageRatio};
    if(this.imageRatio < width/height)
        return {width: height * this.imageRatio, height: height};
    return {width: width, height: height};
}

// Find the various values necessary to properly crop the map to show the given focusBox
Map.prototype.getCrop = function(width, height, focusBox, margin=1.2){
    var fullLong = this.box.maxLong - this.box.minLong;
    var fullLat = this.box.maxLat - this.box.minLat;

    var dLong = focusBox.maxLong - focusBox.minLong;
    var dLat = focusBox.maxLat - focusBox.minLat;

    var out = {};

    var longRatio = fullLong/(dLong*margin);
    var latRatio = fullLat/(dLat*margin);

    if (longRatio < latRatio){
        // Only one zoom, we don't want to stretch the picture.
        out.width = width * longRatio;
        out.height = out.width / this.imageRatio;
    } else {
        out.height = height * latRatio;
        out.width = out.height * this.imageRatio;
    }

    // amount of lat/long margin within the viewport
    var pxPerLat = out.height / fullLat;
    var pxPerLong = out.width / fullLong;
    var marginLong = (width / pxPerLong - dLong) / 2;
    var marginLat = (height / pxPerLat - dLat) / 2;

    out.box = {};
    out.box.minLat = focusBox.minLat - marginLat;
    out.box.maxLat = focusBox.maxLat + marginLat;
    out.box.minLong = focusBox.minLong - marginLong;
    out.box.maxLong = focusBox.maxLong + marginLong;
    
    out.left = (out.box.minLong - this.box.minLong) * pxPerLong;
    out.top = (this.box.maxLat - out.box.maxLat) * pxPerLat;
    return out;
}

Map.belgium = new Map("Belgium", "resource/belgium.svg", 1135.92/987.997, 49.2, 51.77, 2.19, 6.87);

Map.earth = new Map("Earth", "https://upload.wikimedia.org/wikipedia/commons/5/5d/World_map_%28Miller_cylindrical_projection%2C_blank%29.svg", 
    634.26/476.72, -114.5, 112.5, -170, 190);
// Todo: Figure out smarter map-to-dot algorithms for non-mercator projections.

Map.maps = [Map.belgium, Map.earth];


//-------------------//
// Class: Visualizer //
//-------------------//

/// Make a new Visualizer bound to the given elements.
var Visualizer = function(inputSelector, controlSelector, viewSelector){
    // Place our hook into the file selector
    $(inputSelector).on('change', f => readSingleFile(f, this.handleFile.bind(this)));
    // Place our hooks into the controls
    this.initializeControls(controlSelector);
    // Remember our view
    this.$view = $(viewSelector);
}

/// Bind into the controls and set the appropriate events.
Visualizer.prototype.initializeControls = function (controlSelector){
    $c = $(controlSelector);
    var c = this.control = Object();

    // Find the input elements.
    c.$prevDay = $c.find('.prev-day');
    c.$nextDay = $c.find('.next-day');
    c.$range = $c.find('.range-input');
    c.$run = $c.find('.run-input');
    c.$loop = $c.find('.loop-input');
    c.$gradient = $('.gradient-input');

    // Used for enabling-disabling all at once
    c.all = [c.$prevDay, c.$nextDay, c.$range, c.$run, c.$loop, c.$gradient.find("button")];

    // Bind events
    c.$prevDay.on("click", this.prevDay.bind(this));
    c.$nextDay.on("click", this.nextDay.bind(this));
    c.$range.on("input", () => this.updateDay(parseInt(c.$range.val())));
    c.$run.on("change", () => {if(c.$run.prop("checked")) this.run()});

    // Disable the controls, they're not ready yet.
    this.disableControls();
}

/// Automatically step through the different days of the simulation.
Visualizer.prototype.run = async function(){
    this.runSpeed = 60;
    var $run = this.control.$run;
    var $loop = this.control.$loop;

    while($run.prop("checked")){
        var nextDay = this.day + 1;
        if( nextDay >= this.maxDays ){
            if($loop.prop("checked")){
                nextDay = 0;
            } else {
                $run.prop("checked", false);
                break;
            }
        }
        this.updateDay(nextDay);
        await sleep(this.runSpeed);
    }
}

/// Set up and enable the controls based on the simulation data.
Visualizer.prototype.configureControls = function(){
    // Set the slider range
    this.control.$range.prop("max", this.maxDays);

    // Set the different options in the gradient selector
    var $target = this.control.$gradient.find(".dropdown-menu");
    $target.html("");

    // let because we're trying to bind gradient to a lambda later
    for(let gradient in Gradient.gradients){
        var $a = $("<a>");
        $a.text(gradient);

        $a.on("click", () => this.selectGradient(gradient))

        var $li = $("<li>");
        $li.append($a);
        $target.append($li);
    }

    // Enable the controls
    this.disableControls(false);
}

/// Select the gradient by the given name.
Visualizer.prototype.selectGradient = function(name){
    this.gradient = Gradient.gradients[name].scale(this.maxSingle);
    this.updateLegend();
    this.updateMap();
}

/// Control interface methods:
Visualizer.prototype.prevDay = function(){ this.updateDay(visualizer.day - 1); }
Visualizer.prototype.nextDay = function(){ this.updateDay(visualizer.day + 1); }

/// Enable or disable the controls.
Visualizer.prototype.disableControls = function(val=true){
    for(i in this.control.all)
        this.control.all[i].prop("disabled", val);
}

/// Handler passed to the FileReader, called with the contents of the selected file.
Visualizer.prototype.handleFile = function(e) {
    var data = JSON.parse(e.target.result);
    cleanData(data);
    this.initialize(data);
}

/// Actually initialize the Visualizer with the data retrieved from the file.
Visualizer.prototype.initialize = function(data){
    // Grab the data parsed from the file and aggregate it
    this.days = data.days;
    this.towns = data.towns;
    this.aggregateData();

    // Debug method.
    // this.addAlignmentTestNodes();

    // Put the total number of days
    this.maxDays = this.days.length;
    $('.days').text(this.maxDays);
    
    // set up the view
    this.makeView();
    this.updateDay(0);

    // enable the controls
    this.configureControls();
}

/// Extract useful information from our data.
Visualizer.prototype.aggregateData = function(){
    var maxTotal = 0;
    var maxSingle = 0;
    for(d in this.days){
        var total = 0;
        for(t in this.towns){
            var single = this.days[d][t] || 0;
            total += single;
            maxSingle = Math.max(maxSingle, single);
        }
        this.days[d].total = total;
        maxTotal = Math.max(total, maxTotal);
    }
    this.maxTotal = maxTotal;
    this.maxSingle = maxSingle;
}

/// Add a few easily recognisible items to the map for testing alignment of coordinates to the map.
Visualizer.prototype.addAlignmentTestNodes = function(){
    this.towns.append({name:"Tip of tasmania", size:10,lat:-43.597190, long:146.773835});
    this.towns.append({name:"Tip of South America", size:10,lat:-54.671467,long:-65.129066});
    this.towns.append({name:"Tip of Greenland", size:10,lat:59.831982,long:-43.585772});
    this.towns.append({name:"Tip of India", size:10,lat:8.077824, long:77.551041});
}

/// Prepare the HTML document with the basic frameworks for our view.
Visualizer.prototype.makeView = function(){
    this.makeTable();
    this.makeMap();
}

/// Prepare the HTML document with a basic table.
Visualizer.prototype.makeTable = function(){
    // Find and clear the view target
    $target = this.$view.find(".table-view");
    $target.html('');

    // Make a table
    this.$table = $table = $('<table>', {class:'table table-striped table-condensed'});
    $target.append($table);

    // Header
    $row = $('<tr>');
    $row.append($('<th>Name</th>'));
    $row.append($('<th>Inhabitants</th>'));
    $row.append($('<th>Infected</th>'));
    $row.append($('<th>Percentage</th>'));
    $table.append($row);

    // Rows
    for(var i in this.towns){
        var town = this.towns[i];
        $row = $('<tr>', {id:noSpace(town.name)});
        $row.append($('<td>' + town.name + '</td>'));
        $row.append($('<td>' + town.size + '</td>'));
        $row.append($('<td>', {class:'infected'}));
        $row.append($('<td>', {class:'percent'}));
        $table.append($row);
    }
}

/// Initialize the gradient that governs map colouring.
Visualizer.prototype.initializeLegend = function($target){
    this.gradient = Gradient.gradients["Ultra heat map"].scale(this.maxSingle);
    this.$legend = $(".legend-view");
    this.updateLegend();
}

// Updates the legend to match the contents of the selected colour gradient.
Visualizer.prototype.updateLegend = function(){
    this.$legend.html("");
    for(i in this.gradient.colourMap){
        $item = $("<div>", {class: "legend-item"});
        $item.text(Math.round(this.gradient.colourMap[i].val));
        $item.append($("<span>", {class:"legend-circle",style:"background-color:"+this.gradient.colourMap[i].colour.toString()}));
        this.$legend.append($item);
    }
}

/// Prepare the HTML document with a basic table.
Visualizer.prototype.makeMap = function(){
    // Find and clear the view target
    var $target = this.$view.find(".map-view");
    $target.svg("destroy");
    $target.html("");

    this.initializeLegend($target);

    var box = this.findBox();
    var width = 950;
    var height = 800;

    this.sizeFunc = val => Math.sqrt(val) * 8/Math.sqrt(this.maxSingle) + 2;

    // Check if any of the data fits inside a known map.
    for(i in Map.maps){
        map = Map.maps[i];
        if(map.containsBox(box)){
            console.log("Found map: " + map.name);
            // Crop the map
            var o = map.getCrop(width, height, box);
            $crop = $("<div>",{class:"map-image-crop", width:width, height:height});
            $img = $("<img>",{src:map.imageRef, width:o.width, height:o.height});
            $img.css({position: "absolute", top: -o.top, left: -o.left});
            $crop.append($img);
            $target.append($crop);
            box = o.box;
            break;
        }
    }

    // Functions converting lat/longitudes into percentages.
    var latFunc = lat => (lat - box.minLat) / (box.maxLat - box.minLat);
    var longFunc = long => (long - box.minLong) / (box.maxLong - box.minLong);

    // Make an SVG element
    $target.svg({settings: {width:width,height:height}});
    var svgMap = $target.svg("get");

    // Fill it with circles
    for(var i in this.towns){
        var town = this.towns[i];
        // Determine its features
        var x = percentFormat(longFunc(town.long));
        var y = percentFormat(1- latFunc(town.lat));
        var radius = 5;
        var fillColour = "red";

        // Make and attach the circle
        var dot = svgMap.circle(x, y, radius, {fill: fillColour});

        // Remember the circle for later
        town.dot = dot;

        // Add a little tooltip
        dot.setAttribute("title", town.name);
        dot.setAttribute("data-toggle", "tooltip");
        dot.setAttribute("data-container", "body");
    }
    refreshTooltips();
}

// Find the box of latitudes/longitudes containing all known locations.
// Returns an object {minLat, maxLat, minLong, maxLong}
Visualizer.prototype.findBox = function(){
    var out = {minLat: 1000, maxLat: -1000, minLong: 1000, maxLong: -1000};

    for(town in this.towns){
        out.minLat = Math.min(this.towns[town].lat, out.minLat);
        out.maxLat = Math.max(this.towns[town].lat, out.maxLat);
        out.minLong = Math.min(this.towns[town].long, out.minLong);
        out.maxLong = Math.max(this.towns[town].long, out.maxLong);
    }

    return out;
}

/// Update the view to match the info at that day.
Visualizer.prototype.updateDay = function(day){
    // clamp day to the valid range
    day = clamp(day, 0, this.maxDays-1);

    this.day = day;
    $('.current-day').text(1 + day);
    this.control.$range.prop("value", day);
    $('.total-infected').text(this.days[day].total);

    this.updateView();
}

/// Update the view to reflect the currently selected day.
Visualizer.prototype.updateView = function(){
    this.updateTable();
    this.updateMap();
}

/// Update the table to reflect the currently selected day.
Visualizer.prototype.updateTable = function(){
    var currentDay = this.days[this.day];

    // Update each column
    for(town in this.towns){
        var count = currentDay[town] || 0;

        // Find the table column for the given town
        var $col = this.$table.find('#' + noSpace(this.towns[town].name));

        // Put the amount of infected
        $col.find(".infected").text(count);

        // Write the percentage infected if any
        var percent = count/this.towns[town].size;
        $col.find(".percent").text(percent? percentFormat(percent) : '');
    }
}

/// Update the map to reflect the currently selected day.
Visualizer.prototype.updateMap = function(){
    var currentDay = this.days[this.day];

    for(town in this.towns){
        val = currentDay[town] || 0;
        dot = this.towns[town].dot;
        dot.setAttribute("fill", this.gradient.get(val).toString());
        dot.setAttribute("r", this.sizeFunc(val));
    }
}


//------//
// Etc. //
//------//

// Remove all spaces from a given string.
var noSpace = s => s.split(" ").join('');

// Nicely format a given value p as a percentage showing d digits after the period.
var percentFormat = (p, d=1) => (100*p).toFixed(d)+'%';

// Clamp val to [l, r]
var clamp = (val, l, r) => val > r ? r : val < l ? l : val;

// Refresh Bootstrap's tooltips
var refreshTooltips = () => $('[data-toggle="tooltip"]').tooltip(); 

// Await a certain length of time.
var sleep = ms => new Promise(resolve => setTimeout(resolve, ms));