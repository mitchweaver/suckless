const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#d3d3d3", /* black   */
  [1] = "#ef6b7b", /* red     */
  [2] = "#a1d569", /* green   */
  [3] = "#f59335", /* yellow  */
  [4] = "#4ec2e8", /* blue    */
  [5] = "#fec7cd", /* magenta */
  [6] = "#95c1c0", /* cyan    */
  [7] = "#707070", /* white   */

  /* 8 bright colors */
  [8]  = "#b3b3b3", /* black   */
  [9]  = "#ed5466", /* red     */
  [10] = "#afdb80", /* green   */
  [11] = "#f59335", /* yellow  */
  [12] = "#5dc7ea", /* blue    */
  [13] = "#d2a4b4", /* magenta */
  [14] = "#75a1a0", /* cyan    */
  [15] = "#909090", /* white   */

  /* special colors */
  [256] = "#f3f3f3", /* background */
  [257] = "#707070", /* foreground */
};

unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
