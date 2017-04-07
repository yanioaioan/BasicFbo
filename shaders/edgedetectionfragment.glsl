


#version 300 es

precision mediump float;
in vec2 TexCoord;
uniform vec2 pixelSize;
uniform sampler2D Tex1;
layout(location = 0) out vec4 outColor;
float ScreenCoordX=0.5;
uniform float GradientThreshold;
uniform float offset;


float p00, p10, p20, p01, p21, p02, p12, p22, x, y, px , py, distance;
vec3 lum = vec3(0.2126, 0.7152, 0.0722);


//post processing
void main()
{
    //change the color of the texture we previously drawn to..(note we hadn't had set any color to the teapot so all pixelColor r,g,b must be the same)
    vec3 pixelColor = texture(Tex1, TexCoord).rgb;
    if (pixelColor.r==0.0)//whatever was drawn to the texture that we sample from was the teapot without any lights across the surface
    {
        //change the color of the texture we previously drawn to..
        vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
        outColor = color;
        return;
    }
    else if (pixelColor.r>0.9)//whatever was drawn to the texture that we sample from was the teapot without any lights across the surface
    {
        //change the color of the texture we previously drawn to..
        vec4 color = vec4(0.0, 1.0, 0.0, 1.0);
        outColor = color;
        return;
    }



//    if (pixelColor.g==0.0f)
//    {
//        outColor = vec4(0.0);
//        return;
//    }

return;

    //outColor = vec4( (sin(offset)+ 1.0)/2.0, 0,0,1);//rescale to 0..1 instead of -1 1
    //return;

    if(gl_FragCoord.x < ScreenCoordX+1.0 && gl_FragCoord.x > ScreenCoordX-1.0){
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    x = pixelSize.x;
    y = pixelSize.y;
    if(gl_FragCoord.x > ScreenCoordX){
        p00 = dot(texture(Tex1, TexCoord+vec2(-x, y)).rgb, lum);
        p10 = dot(texture(Tex1, TexCoord+vec2(-x,0.)).rgb, lum);
        p20 = dot(texture(Tex1, TexCoord+vec2(-x,-y)).rgb, lum);
        p01 = dot(texture(Tex1, TexCoord+vec2(0., y)).rgb, lum);
        p21 = dot(texture(Tex1, TexCoord+vec2(0.,-y)).rgb, lum);
        p02 = dot(texture(Tex1, TexCoord+vec2( x, y)).rgb, lum);
        p12 = dot(texture(Tex1, TexCoord+vec2( x,0.)).rgb, lum);
        p22 = dot(texture(Tex1, TexCoord+vec2( x,-y)).rgb, lum);

        // Apply Sobel Operator
        px = p00 + 2.0*p10 + p20 - (p02 + 2.0*p12 + p22);
        py = p00 + 2.0*p01 + p02 - (p20 + 2.0*p21 + p22);

        // Check the change in frequency with given threshold
        distance = px*px+py*py;
        if (distance > GradientThreshold ){
            outColor = vec4(0.0, 0.0, 0.0, 1.0);

        }
        else{
            outColor = vec4(1.0);

        }
    }
    else
    {
        outColor = texture(Tex1, TexCoord);
    }
    return;

}

