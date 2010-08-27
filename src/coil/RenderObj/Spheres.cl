__kernel void
SphereRenderKernel(const __global float * positions,
		   __global float * vertexBuffer,
		   const __global float * vertices,
		   const uint nVertex,
		   const uint StartParticle,
		   const uint EndParticle,
		   const int VertexOffset)
{
  //We're trying to coalesce the reads and writes from memory, so we cache
  __local float positionCache[WORKGROUP_SIZE];
  
  //Iterate on blocks of WORKGROUP_SIZE, with a stride of the global_size
  for (uint id0 = get_group_id(0) * WORKGROUP_SIZE + 3*StartParticle;
       id0 < 3 * EndParticle; 
       id0 += get_global_size(0))
    {
      //Load the cache
      uint data_to_read = id0 + get_local_id(0);
      
      //Coalesced read from global mem and conflict-free localsss write
      if (data_to_read < 3 * EndParticle)
	positionCache[get_local_id(0)] = positions[data_to_read];
      
      barrier(CLK_LOCAL_MEM_FENCE);
      
      //Cache is full, now start blitting out verticies
      //Calculate the number of verticie components to do
      uint particles_comp_InCache = min((uint)(WORKGROUP_SIZE), 3*EndParticle - id0);
      uint vertices_components_to_do = particles_comp_InCache * nVertex ;
      
      for (uint vertex_component = get_local_id(0);
	   vertex_component < vertices_components_to_do;
	   vertex_component += get_local_size(0))
	{
	  uint particleID = 3*(vertex_component / (3 * nVertex)) + (vertex_component % 3);
	  uint vertexID = vertex_component % (3*nVertex);
	  
	  vertexBuffer[VertexOffset + nVertex * id0 + vertex_component]
	    = positionCache[particleID] + vertices[vertexID];
	}

      barrier(CLK_LOCAL_MEM_FENCE);
    }
}

