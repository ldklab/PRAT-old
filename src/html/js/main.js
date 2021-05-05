/*******************************************************************************
 **
 ** main.js
 **
 ** Main file for DOM handlers.
 **
 */

 (function (main) {

    "use strict";
    let __module__ = main.__module__ = "[main]";
  
    /***************************************************************************
     ** Dependency check.
     */
  
    if (typeof jQuery === "undefined") console.log(__module__, " could not load jQuery");
    //if (typeof d3 === "undefined") console.log(__module__, " could not load d3");

    main.initTable = (function() {
        console.log("[+] Initialising table");

        let myTable = $("#scBody");

        // Fill table with links to coverage files for now.
        myTable.append("<tr><td><a href=\"../../reports/FILE\" target=\"_blank\">Source file</td></tr>");
    });
  
  })(typeof exports !== 'undefined' ? exports : (this.main = {}));
  /* eof */