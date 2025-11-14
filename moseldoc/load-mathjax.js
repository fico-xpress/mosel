// File load-mathjax.js
//  -- Mathjax configuration for Xpress HTML documentation --
//
//  Copyright (c) 2021 Fair Isaac Corporation. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
window.MathJax = {
  tex: {
    inlineMath: [['\\(', '\\)']],
    tags: 'ams',            // enable eq numbering 
    macros: {
      bbbr: "{\\rm I\\! R}",
      bbbn: "{\\rm I\\! N}",
      amp: "&amp;",
      tamp: "\&"            // table column separator
    }
  },
};
(function () {
  var script = document.createElement('script');
  script.src = 'https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-chtml.js';
  script.async = true;
  document.head.appendChild(script);
})();
