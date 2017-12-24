/* prototype of a Space object
   the space object provides methods to draw a 3d coordinatesystem on a canvas
   2d methods use coordinates between 0 and canvas.width or canvas.height
   3d methods use coordinates between -1.0 and 1.0 (for the longest axis)
@canvas: the canvas to draw on */
function Space(canvas) {
	/* save the canvas as property */
	this.canvas = canvas;
	/* get the context as property */
	this.context = canvas.getContext("2d");
	/* calculate some frequently used constants */
	this.DIAGONAL = Math.sqrt(canvas.width * canvas.width + canvas.height * canvas.height);
	this.SQRT2 = Math.sqrt(2);
	
	/* draws a 2d dotted line on the canvas
	@xStart: x coordinate to start the line
	@yStart: y coordinate to start the line
	@xStop: x coordinate to stop the line
	@yStop: y coordinate to stop the line
	@dotLength: length of the dot(line), alternate line <-> empty */
	this.dottedLine = function(xStart, yStart, xStop, yStop, dotLength) {
		this.context.save();
		this.context.beginPath();
		var xDist = xStop - xStart;
		var yDist = yStop - yStart;
		var dots = Math.floor(Math.sqrt(xDist * xDist + yDist * yDist) / dotLength);
		dots += (dots % 2 == 0) ? 1 : 0;
		var x, y;
		for(var d = 0; d < dots; d += 2) {
			x = xStart + d * xDist / dots;
			y = yStart + d * yDist / dots;
			this.context.moveTo(x, y);
			this.context.lineTo(x + xDist / dots, y + yDist / dots);
		}
		this.context.closePath();
		this.context.stroke();
		this.context.restore();
	}
	
	/* draws a 3d point and a dotted dice to visualize it better on the canvas
	@x: x coordinate of the point (-1.0 to 1.0)
	@y: y coordinate of the point (-1.0 to 1.0)
	@z: z coordinate of the point (-1.0 to 1.0) */
	this.drawPoint = function(x, y, z) {
		this.context.save();
		var RADIUS = this.DIAGONAL / 100;
		var DOT_LENGTH = 10;
		var length  = (this.canvas.width > this.canvas.height) ? this.canvas.width : this.canvas.height;
		x *= length / 2;
		y *= length / 2;
		z *= length / 2 / this.SQRT2;
		
		this.context.translate(canvas.width / 2, canvas.height / 2);
		this.context.scale(1, -1);
		this.context.strokeStyle = "#FF000077";
		this.context.lineWidth = 3;
		
		this.dottedLine(0, 0, x, 0, DOT_LENGTH);
		this.dottedLine(0, 0, 0, y, DOT_LENGTH);
		this.dottedLine(0, 0, z, z, DOT_LENGTH);
		this.dottedLine(x, 0, x, y, DOT_LENGTH);
		this.dottedLine(x, 0, x + z, z, DOT_LENGTH);
		this.dottedLine(0, y, x, y, DOT_LENGTH);
		this.dottedLine(0, y, z, y + z, DOT_LENGTH);
		this.dottedLine(z, y + z, z, z, DOT_LENGTH);
		this.dottedLine(z, y + z, x + z, y + z, DOT_LENGTH);
		this.dottedLine(x, y, x + z, y + z, DOT_LENGTH);
		this.dottedLine(x + z, y + z, x + z, z, DOT_LENGTH);
		this.dottedLine(x + z, z, z, z, DOT_LENGTH);
		
		var gradient = this.context.createRadialGradient(x + z, y + z, RADIUS, x + z, y + z, 0);
		gradient.addColorStop(0, "#00FF00");
		gradient.addColorStop(1, "#FFFFFF");
		this.context.fillStyle = gradient;
		this.context.beginPath();
		this.context.arc(x + z, y + z, RADIUS, 0, 2 * Math.PI);
		this.context.closePath();
		this.context.fill();
		this.context.restore();
	}
	
	/* draws the x, y and z axis on the canvas*/
	this.drawAxis = function() {
		this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
		var FONT_SIZE =  this.DIAGONAL / 50;
		var TICK_LENGTH = this.DIAGONAL / 100;
		var PIXELS_PER_TICK = this.DIAGONAL / 20;
	
		this.context.save();
		this.context.beginPath();
		var ticks, offset;

		this.context.lineWidth = this.DIAGONAL / 500;
		this.context.textAlign = "center";
		this.context.font = FONT_SIZE + "px Comic Sans MS";
		//draws the x axis
		this.context.moveTo(0, canvas.height / 2);
		this.context.lineTo(canvas.width, canvas.height / 2);
		this.context.fillText("x", FONT_SIZE, canvas.height / 2 - FONT_SIZE);
		ticks = Math.floor(canvas.width / PIXELS_PER_TICK);
		offset = canvas.width % PIXELS_PER_TICK / 2;
		if(ticks % 2 == 1)
			offset += PIXELS_PER_TICK / 2;
		for(var t = 0; t <= ticks; t++) {
			this.context.moveTo(offset + t * PIXELS_PER_TICK, (canvas.height - TICK_LENGTH) / 2);
			this.context.lineTo(offset + t * PIXELS_PER_TICK, (canvas.height + TICK_LENGTH) / 2);
		}
		//draws the y axis
		this.context.moveTo(canvas.width / 2, 0);
		this.context.lineTo(canvas.width / 2, canvas.height);
		this.context.fillText("y", canvas.width / 2 + FONT_SIZE, FONT_SIZE);
		ticks = Math.floor(canvas.height / PIXELS_PER_TICK);
		offset = (canvas.height % PIXELS_PER_TICK) / 2;
		if(ticks % 2 == 1)
			offset += PIXELS_PER_TICK / 2;
		for(var t = 0; t <= ticks; t++) {
			this.context.moveTo((canvas.width - TICK_LENGTH) / 2, offset + t * PIXELS_PER_TICK);
			this.context.lineTo((canvas.width + TICK_LENGTH) / 2, offset + t * PIXELS_PER_TICK);
		}
		//draws the z axis
		if(canvas.width > canvas.height) {
			this.context.moveTo((canvas.width - canvas.height) / 2, canvas.height);
			this.context.lineTo((canvas.width + canvas.height) / 2, 0);
			this.context.fillText("z", (canvas.width + canvas.height) / 2 + FONT_SIZE, FONT_SIZE);
			var offsetY, x, y;
			ticks = Math.floor(this.SQRT2 * canvas.height / PIXELS_PER_TICK);
			offset = (canvas.width - canvas.height + this.SQRT2 * canvas.height % PIXELS_PER_TICK / this.SQRT2) / 2;
			offsetY = canvas.height - this.SQRT2 * canvas.height % PIXELS_PER_TICK / this.SQRT2 / 2;
			if(ticks % 2 == 1) {
				offset += PIXELS_PER_TICK / this.SQRT2 / 2;
				offsetY -= PIXELS_PER_TICK / this.SQRT2 / 2;
			}
			for(var t = 0; t <= ticks; t++) {
				x = offset + t * PIXELS_PER_TICK / this.SQRT2;
				y = offsetY - t * PIXELS_PER_TICK / this.SQRT2;
				this.context.moveTo(x - TICK_LENGTH / this.SQRT2 / 2, y - TICK_LENGTH / this.SQRT2 / 2);
				this.context.lineTo(x + TICK_LENGTH / this.SQRT2 / 2, y + TICK_LENGTH / this.SQRT2 / 2);
			}
		} else {
			this.context.moveTo(canvas.width, (canvas.height - canvas.width) / 2);
			this.context.lineTo(0, (canvas.height + canvas.width) / 2);
			this.context.fillText("z", FONT_SIZE, (canvas.height + canvas.width) / 2 + FONT_SIZE);
			var offsetX, x, y;
			ticks = Math.floor(this.SQRT2 * canvas.width / PIXELS_PER_TICK);
			offset = (canvas.height + canvas.width - this.SQRT2 * canvas.width % PIXELS_PER_TICK / this.SQRT2) / 2;
			offsetX = this.SQRT2 * canvas.width % PIXELS_PER_TICK / this.SQRT2 / 2;
			if(ticks % 2 == 1) {
				offset -= PIXELS_PER_TICK / this.SQRT2 / 2;
				offsetX += PIXELS_PER_TICK / this.SQRT2 / 2;
			}
			for(var t = 0; t <= ticks; t++) {
				x = offsetX + t * PIXELS_PER_TICK / this.SQRT2;
				y = offset - t * PIXELS_PER_TICK / this.SQRT2;
				this.context.moveTo(x - TICK_LENGTH / this.SQRT2 / 2, y - TICK_LENGTH / this.SQRT2 / 2);
				this.context.lineTo(x + TICK_LENGTH / this.SQRT2 / 2, y + TICK_LENGTH / this.SQRT2 / 2);
			}
		}
		this.context.closePath();
		this.context.stroke();
		this.context.restore();
	}
	
}
