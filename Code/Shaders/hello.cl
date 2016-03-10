typedef struct tagray
{
	float4 origin;
	float4 direction;
}Ray;

__kernel void hello(__global float* output, __global float* balls, __global float* cameraData, __global float* imageData, __global float* triangleData)
{
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	const int w = get_global_size(0);
	const int h = get_global_size(1);

	float4 lightPos = (float4)(0, 0, -70, 0);
	float4 lightColour = (float4)(1, 1, 1, 0);
	
	Ray r;
	r.origin = (float4)(cameraData[0], cameraData[1], cameraData[2], 0.f);
	r.direction = (float4)(i - (float)w * .5f, -j + (float)h * 0.5f, h * (w / h), 0.f);
	
	float4 rightDir = (float4)(cameraData[4], cameraData[5], cameraData[6], cameraData[7]);
	float4 viewDir = (float4)(cameraData[8], cameraData[9], cameraData[10], cameraData[11]);
	float4 upDir = cross(viewDir, rightDir);
	
	r.direction = r.direction.x * rightDir + r.direction.y * upDir + r.direction.z * viewDir;
	r.direction = normalize(r.direction);
	
	float closestT = FLT_MAX;
	float4 ambientColour = (float4)(0, 0, 0, 0);
	float4 diffuseColour = (float4)(0, 0, 0, 0);
	float4 specularColour = (float4)(0, 0, 0, 0);
	float4 reflectionVector = (float4)(0, 0, 0, 0);
	float4 rVector = (float4)(0, 0, 0, 0);
	float4 hitpos = (float4)(0, 0, 0, 0);

	output[(i + j * w) * 3] = 0;
	output[(i + j * w) * 3 + 1] = 0;
	output[(i + j * w) * 3 + 2] = 0;

	for (int zzzz = 1; zzzz <= 4; zzzz *= 2)
	{
		/**--**--**--**--**--**--**/
		/*          WALLS         */
		/**--**--**--**--**--**--**/
		/*
		float tempT = 0;
		if (r.direction.x > 0 && r.origin.x <= 100) //right plane
		{
			tempT = (100 - r.origin.x) / r.direction.x;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.y < 100 && hitpos.y > -100 && hitpos.z < 100 && hitpos.z > -100) // will be able to hit the right wall
			{

				int texPixX = floor(((hitpos.z + 100) / 200) * 22); // (XPos, + halfplanewidth) / planewidth * immagewidth
				int texPixY = floor(((hitpos.y + 100) / 200) * 18);
				float4 pixCol = (float4)(imageData[(texPixX + texPixY * 22) * 3], imageData[(texPixX + texPixY * 22) * 3 + 1], imageData[(texPixX + texPixY * 22) * 3 + 2], 0);
				closestT = length(r.direction * tempT);
				float4 N = (float4)(-1, 0, 0, 0);
				float lightInt = dot(N, normalize(lightPos - hitpos));

				reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

				rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
				ambientColour = 0.2f * pixCol; // ambient light 
				//diffuseColour = 0.4f * lightInt * pixCol;
				//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * pixCol;
			}
		}
		else if (r.direction.x < 0 && r.origin.x >= -100)// left plane
		{
			tempT = -(100 + r.origin.x) / r.direction.x;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.y < 100 && hitpos.y > -100 && hitpos.z < 100 && hitpos.z > -100) // will be able to hit the right wall
			{
				int texPixX = floor(((hitpos.z + 100) / 200) * 22); // (XPos, + halfplanewidth) / planewidth * immagewidth
				int texPixY = floor(((hitpos.y + 100) / 200) * 18);
				float4 pixCol = (float4)(imageData[(texPixX + texPixY * 22) * 3], imageData[(texPixX + texPixY * 22) * 3 + 1], imageData[(texPixX + texPixY * 22) * 3 + 2], 0);
				closestT = length(r.direction * tempT);
				float4 N = (float4)(1, 0, 0, 0);
				float lightInt = dot(N, normalize(lightPos - hitpos));

				reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

				rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
				ambientColour = 0.2f * pixCol; // ambient light 
				//diffuseColour = 0.4f * lightInt * pixCol;
				//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * pixCol;
			}
		}
		if (r.direction.y > 0 && r.origin.y <= 100) //down plane
		{
			tempT = (100 - r.origin.y) / r.direction.y;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.x < 100 && hitpos.x > -100 && hitpos.z < 100 && hitpos.z > -100) // will be able to hit the right wall
			{
				tempT = length(r.direction * tempT);
				if (tempT < closestT)
				{
					closestT = tempT;
					float4 N = (float4)(0, -1, 0, 0);
					float lightInt = dot(N, normalize(lightPos - hitpos));

					reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

					rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
					ambientColour = (float4)(0.2f, 0.2f, 0.2f, 0); // ambient light 
					//diffuseColour = 0.4f * lightInt * (float4)(1, 1, 1, 0);
					//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * (float4)(1, 1, 1, 0);
				}
			}
		}
		else if (r.direction.y < 0 && r.origin.y >= -100)// up plane
		{
			tempT = -(100 + r.origin.y) / r.direction.y;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.x < 100 && hitpos.x > -100 && hitpos.z < 100 && hitpos.z > -100) // will be able to hit the right wall
			{
				tempT = length(r.direction * tempT);
				if (tempT < closestT)
				{
					closestT = tempT;
					float4 N = (float4)(0, 1, 0, 0);
					float lightInt = dot(N, normalize(lightPos - hitpos));

					reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

					rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
					ambientColour = (float4)(0.2f, 0.2f, 0.2f, 0); // ambient light 
					//diffuseColour = 0.4f * lightInt * (float4)(1, 1, 1, 0);
					//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * (float4)(1, 1, 1, 0);
				}
			}
		}
		if (r.direction.z > 0 && r.origin.z <= 100) //front plane
		{
			tempT = (100 - r.origin.z) / r.direction.z;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.x < 100 && hitpos.x > -100 && hitpos.y < 100 && hitpos.y > -100) // will be able to hit the right wall
			{
				tempT = length(r.direction * tempT);
				if (tempT < closestT)
				{
					closestT = tempT;
					float4 N = (float4)(0, 0, -1, 0);
					float lightInt = dot(N, normalize(lightPos - hitpos));

					reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

					rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
					ambientColour = (float4)(0.2f, 0.2f, 0.2f, 0); // ambient light 
					//diffuseColour = 0.4f * lightInt * (float4)(1, 1, 1, 0);
					//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * (float4)(1, 1, 1, 0);
				}
			}
		}
		else if (r.direction.z < 0 && r.origin.z >= -100)// back plane
		{
			tempT = -(100 + r.origin.z) / r.direction.z;
			hitpos = r.origin + r.direction * tempT;
			if (hitpos.x < 100 && hitpos.x > -100 && hitpos.y < 100 && hitpos.y > -100) // will be able to hit the right wall
			{
				tempT = length(r.direction * tempT);
				if (tempT < closestT)
				{
					closestT = tempT;
					float4 N = (float4)(0, 0, 1, 0);
					float lightInt = dot(N, normalize(lightPos - hitpos));

					reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);

					rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);
					ambientColour = (float4)(0.2f, 0.2f, 0.2f, 0); // ambient light 
					//diffuseColour = 0.4f * lightInt * (float4)(1, 1, 1, 0);
					//specularColour = 0.3f * lightInt * max(pow(dot(reflectionVector, r.direction), 6), 0.f) * (float4)(1, 1, 1, 0);
				}
			}
		}
		*/
		/**--**--**--**--**--**--**/
		/*          BALLS         */
		/**--**--**--**--**--**--**/

		int numballs = balls[0];
		float4 ballcenter = (float4)(0, 0, 0, 0);
		for (int k = 0; k < numballs; k++)
		{
			ballcenter.x = balls[k * 4 + 1];
			ballcenter.y = balls[k * 4 + 2];
			ballcenter.z = balls[k * 4 + 3];
			ballcenter.w = 0.0f;
			float radius = balls[k * 4 + 4];


			float4 L = ballcenter - r.origin;
			float tca = dot(L, r.direction);
			if (tca > 0)
			{
				float d = sqrt(dot(L, L) - tca * tca);

				if (d > 0 && d <= radius)
				{
					float thc = sqrt(radius * radius - d * d);
					float t0 = tca - thc;
					float t1 = tca + thc;
					if (t0 <= t1 && t0 < closestT)
					{
						hitpos = r.origin + t0 * r.direction; // intersection point
						float4 N = hitpos - ballcenter;
						N = normalize(N); // normal at intersection
						float lightInt = dot(N, normalize(lightPos - hitpos));

						reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N,N) * dot(N,N)) * N);

						rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);

						float4 pixCol = (float4)(imageData[k * 3], imageData[k * 3 + 1], imageData[k * 3 + 2], 0);

						ambientColour = 0.5f * pixCol; // ambient light 
						diffuseColour = (float4)(0, 0, 0, 0);
						//diffuseColour = 0.5f * lightInt * pixCol; // diffuse light intensity
						specularColour = 0.2f * lightInt * max(pow(dot(reflectionVector, r.direction), 4), 0.f) * pixCol;
						closestT = t0;
					}
					else if (t0 > t1 && t1 < closestT)
					{
						hitpos = r.origin + t1 * r.direction;
						float4 N = hitpos - ballcenter;
						N = normalize(N); // normal at intersection
						float lightInt = dot(N, normalize(lightPos - hitpos));
						reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);
						rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);

						float4 pixCol = (float4)(imageData[k * 3], imageData[k * 3 + 1], imageData[k * 3 + 2], 0);

						ambientColour = 0.5f * pixCol; // ambient light 
						diffuseColour = (float4)(0, 0, 0, 0);
						//diffuseColour = 0.5f * lightInt * pixCol; // diffuse light intensity
						specularColour = 0.2f * lightInt * max(pow(dot(reflectionVector, r.direction), 4), 0.f) * pixCol;
						closestT = t1;
					}
				}
			}
		}

		/**--**--**--**--**--**--**/
		/*     TRAIANGLES         */
		/**--**--**--**--**--**--**/
		int nTriangles = triangleData[0];
		for (int k = 0; k < nTriangles; k++)
		{
			float4 v0 = (float4)(triangleData[k * 24 + 1], triangleData[k * 24 + 2], triangleData[k * 24 + 3], 0);
			float4 v1 = (float4)(triangleData[k * 24 + 9], triangleData[k * 24 + 10], triangleData[k * 24 + 11], 0);
			float4 v2 = (float4)(triangleData[k * 24 + 17], triangleData[k * 24 + 18], triangleData[k * 24 + 19], 0);

			float4 A = v1 - v0; // edge 0
			float4 B = v2 - v0; // edge 1
			float4 N = cross(A, B); // normal ( will be sent in with the model for each vertice? to make it smoother)
			N = normalize(N);

			float D = dot(N, v0); // plane distance from O
			float ndotdir = dot(N, r.direction);
			if (ndotdir != 0) // if ray is parallell with triangle, no intersection
			{
				float triT = -(dot(N, r.origin) - D) / ndotdir;
				if (triT > 0 && triT < closestT) // if triangle is behind camera, no intersection. triT closest, worth a try.
				{
					float4 Thitpos = r.origin + triT * r.direction; // ray will hit the plane spanned by triangle here

					// inside-outside test
					B = v2 - v1;
					float4 C = v0 - v2;

					float4 C0 = Thitpos - v0;
					float4 C1 = Thitpos - v1;
					float4 C2 = Thitpos - v2;

					if (dot(N, cross(A, C0)) >= 0 && dot(N, cross(B, C1)) >= 0 && dot(N, cross(C, C2)) >= 0) // triangle is hit! Good job
					{
						hitpos = Thitpos;
						float2 t0 = (float2)(triangleData[k * 24 + 4], triangleData[k * 24 + 5]);
						float2 t1 = (float2)(triangleData[k * 24 + 12], triangleData[k * 24 + 13]);
						float2 t2 = (float2)(triangleData[k * 24 + 20], triangleData[k * 24 + 21]);

						float4 n0 = (float4)(triangleData[k * 24 + 6], triangleData[k * 24 + 7], triangleData[k * 24 + 8], 0);
						float4 n1 = (float4)(triangleData[k * 24 + 14], triangleData[k * 24 + 15], triangleData[k * 24 + 16], 0);
						float4 n2 = (float4)(triangleData[k * 24 + 22], triangleData[k * 24 + 23], triangleData[k * 24 + 24], 0);

						float4 f0 = v0 - hitpos;
						float4 f1 = v1 - hitpos;
						float4 f2 = v2 - hitpos;
						float a = length(cross(v0 - v1, v0 - v2));
						float a1 = length(cross(f1, f2)) / a;
						float a2 = length(cross(f2, f0)) / a;
						float a3 = length(cross(f0, f1)) / a;

						float u = a1 * t0.x + a2 * t1.x + a3 * t2.x;
						float v = a1 * t0.y + a2 * t1.y + a3 * t2.y;

						int texX = floor(u * 22);
						int texY = floor(v * 18);

						N.x = a1 * n0.x + a2 * n1.x + a3 * n2.x;
						N.y = a1 * n0.y + a2 * n1.y + a3 * n2.y;
						N.z = a1 * n0.z + a2 * n1.z + a3 * n2.z;

						N = normalize(n0);
						float lightInt = dot(N, normalize(lightPos - hitpos));
						reflectionVector = normalize(normalize(lightPos - hitpos) - 2 * (dot(normalize(lightPos - hitpos), N) / dot(N, N) * dot(N, N)) * N);
						rVector = normalize(r.direction - 2 * (dot(r.direction, N) / dot(N, N) * dot(N, N)) * N);

						float4 pixCol = (float4)(imageData[(texX + texY * 22) * 3], imageData[(texX + texY * 22) * 3 + 1], imageData[(texX + texY * 22) * 3 + 2], 0);

						ambientColour = 0.3f * pixCol;
						diffuseColour = 0.5f * lightInt * pixCol; // diffuse light intensity
						specularColour = 0.2f * lightInt * max(pow(dot(reflectionVector, r.direction), 4), 0.f) * pixCol;
						closestT = triT;
					}
				}
			}
		}
		output[(i + j * w) * 3] += clamp((ambientColour.x + diffuseColour.x + specularColour.x) * (0.8f / (zzzz )), 0.0, 1.0);
		output[(i + j * w) * 3 + 1] += clamp((ambientColour.y + diffuseColour.y + specularColour.y) * (0.8f / (zzzz )), 0.0, 1.0);
		output[(i + j * w) * 3 + 2] += clamp((ambientColour.z + diffuseColour.z + specularColour.z) * (0.8f / (zzzz )), 0.0, 1.0);

		r.direction = rVector;
		r.origin = hitpos + rVector * 0.001;
		closestT = FLT_MAX;
		ambientColour = (float4)(0, 0, 0, 0);
		diffuseColour = (float4)(0, 0, 0, 0);
		specularColour = (float4)(0, 0, 0, 0);
	}
	output[(i + j * w) * 3] = clamp(output[(i + j * w) * 3], 0.0, 1.0);
	output[(i + j * w) * 3 + 1] = clamp(output[(i + j * w) * 3 + 1], 0.0, 1.0);
	output[(i + j * w) * 3 + 2] = clamp(output[(i + j * w) * 3 + 2], 0.0, 1.0);
}